const { spawn } = require("child_process");
const { copyFileSync } = require("fs");
ip="192.168.0.105"
module.exports = async (router,MediasoupVideoSrcPort,MediasoupAudioSrcPort) => {
  const audioTransport = await router.createPlainRtpTransport({
    listenIp: ip,
    rtcpMux: false,
    comedia: true
  });

  const videoTransport = await router.createPlainRtpTransport({
    listenIp: ip,
    rtcpMux: false,
    comedia: true
  });



  const audioProducer = await audioTransport.produce({
    kind: "audio",
    rtpParameters: {
      codecs: [
        {
          mimeType: "audio/opus",
          clockRate: 48000,
          payloadType: 101,
          channels: 2,
          rtcpFeedback: [],
          parameters: {},
        }
      ],
      encodings: [{ ssrc: 1 }]
    }
  });

  const videoProducer = await videoTransport.produce({
    kind: "video",
    rtpParameters: {
      codecs: [
        {
          mimeType: "video/H264",
          clockRate: 90000,
          payloadType: 102,
          rtcpFeedback: [],
          parameters: {
            "packetization-mode": 1,
            "profile-level-id": "42e01f"
          }
        }
      ],
      //rtcp:{cname:"1003"},
      encodings: [{ ssrc: 2 }]
    }
  });
  console.log("recive back rtp port",videoTransport.tuple.localPort)
  console.log("recive back rtcp port",videoTransport.rtcpTuple.localPort)
console.log("audio rtp", audioTransport.tuple.localPort,
  "audio rtcp",audioTransport.rtcpTuple.localPort,
  "video rtp",videoTransport.tuple.localPort,
  "video rtcp",videoTransport.rtcpTuple.localPort,
  "media src port video",MediasoupVideoSrcPort,
  
  "media src port audio",MediasoupAudioSrcPort)
 //console.log("cname is " ,videoProducer)
  spawnGstRtpInputSource({
    host: ip,
    audioRtpPort: audioTransport.tuple.localPort,
    audioRtcpPort: audioTransport.rtcpTuple.localPort,
    videoRtpPort: videoTransport.tuple.localPort,
    videoRtcpPort: videoTransport.rtcpTuple.localPort,
    MediasoupVideoSrcPort:MediasoupVideoSrcPort,
    MediasoupAudioSrcPort:MediasoupAudioSrcPort
  });

  return {
    audioProducerId: audioProducer.id,
    videoProducerId: videoProducer.id,
    audioRtpPort: audioTransport.tuple.localPort,
    audioRtcpPort: audioTransport.rtcpTuple.localPort,
    videoRtpPort: videoTransport.tuple.localPort,
    videoRtcpPort: videoTransport.rtcpTuple.localPort,
    MediasoupVideoSrcPort:MediasoupVideoSrcPort,
    MediasoupAudioSrcPort:MediasoupAudioSrcPort
  };
};

const spawnGstRtpInputSource = ({
  host,
  audioRtpPort,
  audioRtcpPort,
  videoRtpPort,
  videoRtcpPort,
  MediasoupVideoSrcPort,
  MediasoupAudioSrcPort
}) => {
  const command = "GST_DEBUG=3  lib/sendrecrtp";
  const args = [
    `${MediasoupVideoSrcPort}`,//
    `${videoRtpPort}`,
    `${videoRtcpPort}`,
    `${MediasoupAudioSrcPort}`,
    `${audioRtpPort}`,
    `${audioRtcpPort}`
  ].map(a => a.split(" "))
    .flat();

   this._process=spawn(command, args,{
    detached: false,
    shell: true
  });

      if (this._process.stderr) {
      this._process.stderr.setEncoding('utf-8');
    }

    if (this._process.stdout) {
      this._process.stdout.setEncoding('utf-8');
    }

    this._process.on('message', message =>
      console.log('gstreamer::process::message [pid:%d, message:%o]', this._process.pid, message)
    );

    this._process.on('error', error =>
      console.error('gstreamer::process::error [pid:%d, error:%o]', this._process.pid, error)
    );


    this._process.stderr.on('data', data =>{
      console.log('gstreamer::process::stderr::data [data:%o]', data)
    }
    );

    this._process.stdout.on('data', data =>{
         console.log('gstreamer::process::stdout::data [data:%o]', data)

    }
    );
  
};
