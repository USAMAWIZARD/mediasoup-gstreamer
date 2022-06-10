const { stat } = require("fs");
const http = require("http");
const { createWorker } = require("mediasoup");
const produceRtp = require("./lib/produce-rtp");
ip="192.168.0.105"

producers=[]
const mediaCodecs = [
  {
    kind: "video",
    name: "H264",
    mimeType: "video/H264",
    clockRate: 90000,
    parameters: {
      "packetization-mode": 1,
      "profile-level-id": "42e01f"
    }
  },
  {
    kind: "audio",
    mimeType: "audio/opus",
    clockRate: 48000,
    channels: 2
  }
];
const port = 3000;


(async () => {
  const worker = await createWorker();
  const router = await worker.createRouter({ mediaCodecs });
  console.log("router:", router.id);


  const transportMap = new Map();
  http
    .createServer(async (req, res) => {
      res.writeHead(200, {
        "Content-Type": "application/json",
        "Access-Control-Allow-Origin": "*"
      });

      const [url, qs] = req.url.split("?q=");
      const query = qs ? JSON.parse(decodeURIComponent(qs)) : {};

      console.log(`[req]: ${url}`);
      switch (url) {
        case "/rtpCapabilities": {
          res.end(JSON.stringify(router.rtpCapabilities));
          break;
        }
        case "/createTransport": {
          const transport = await router.createWebRtcTransport({ listenIps: [ip] });
          transportMap.set(transport.id, transport);

          res.end(
            JSON.stringify({
              id: transport.id,
              iceParameters: transport.iceParameters,
              iceCandidates: transport.iceCandidates,
              dtlsParameters: transport.dtlsParameters
            })
          );
          break;
        }
        case "/transportConnect": {
          const { transportId, dtlsParameters } = query;
          const transport = transportMap.get(transportId);
          await transport.connect({ dtlsParameters });

          res.end(JSON.stringify({}));
          break;
        }
        case "/produce":
         MediasoupVideoSrcPort=Math.floor(Math.random()*(8000-5000+1)+5000)
         MediasoupVideoRTCPPort=Math.floor(Math.random()*(8000-5000+1)+5000)
         MediasoupAudioSrcPort=Math.floor(Math.random()*(8000-5000+1)+5000)
         MediasoupAudioRTCPPort=Math.floor(Math.random()*(8000-5000+1)+5000)
         query.videoProducerData.rtpPort=MediasoupVideoSrcPort
         query.videoProducerData.rtcpPort=MediasoupVideoRTCPPort
         query.audioProducerData.rtpPort=MediasoupAudioSrcPort
         query.audioProducerData.rtcpPort=MediasoupAudioRTCPPort

         var gstTransportInfo = await produceRtp(router,MediasoupVideoSrcPort,MediasoupAudioSrcPort);
         console.log('audio prod id',gstTransportInfo.audioProducerId,'\nvideo prod id', gstTransportInfo.videoProducerId );
       
         //  query.rtpParameters["rtcp"]={cname:"1003"}
          //  query.rtpParameters.rtcp={ cname: '48c90612-12ad-4bd2-addb-de165febd4f1'+parseInt(Math.random()*5), reducedSize: true }
          //  console.log(query.rtpParameters["rtcp"])
      
            const transport = transportMap.get(query.videoProducerData.transportId);
      [query.videoProducerData,query.audioProducerData].forEach(async(producerData)=>{ 
            console.log(producerData.kind)
            console.log("rtp port",producerData.rtpPort)
            console.log("rtcp port",producerData.rtcpPort)
             producer = await transport.produce({
              kind: producerData.kind,
              rtpParameters:producerData.rtpParameters 
            });
            producerData.producer=producer
            console.log(producerData.kind," without gstremer",producer.id)
 

              const videoConsumeTransport = await router.createPlainRtpTransport({
              listenIp: ip,
              rtcpMux: false,
              comedia: false
            });
         
            await videoConsumeTransport.connect({
              ip: '192.168.0.105',
              port: producerData.rtpPort,
              rtcpPort: producerData.rtcpPort})
              console.log(producerData.producer.id)
                const rtpConsumer = await videoConsumeTransport.consume({
                producerId: producerData.producer.id,
                rtpCapabilities:router.rtpCapabilities,
                paused: false
              });
            
           setInterval(async()=>{
            a= await videoConsumeTransport.getStats()
          //  console.log(a)
           },2000)

          })
          res.end(JSON.stringify({}));

             // console.log(router.rtpCapabilities)
        
        break
        case "/consume": {
          const { transportId, producerId, rtpCapabilities } = query;
          const transport = transportMap.get(transportId);
          const consumer = await transport.consume({
            producerId,
            rtpCapabilities
          });

          res.end(
            JSON.stringify({
              id: consumer.id,
              producerId,
              kind: consumer.kind,
              rtpParameters: consumer.rtpParameters
            })
          );
          break;
        }
        default:
          console.error("N/A route", url);
      }
    })
    .listen(port);

  console.log("server started at port", port);
})();
