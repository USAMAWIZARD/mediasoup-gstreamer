/* eslint-disable require-atomic-updates */
import { Device } from 'mediasoup-client';

const request = (path, query) => {
  const qs = query ? "?q=" + encodeURIComponent(JSON.stringify(query)) : "";
  return fetch(`http://`+window.location.hostname+`:3000/${path}${qs}`).then(res => res.json());
};

const $consume = document.getElementById("consume");
const $pid = document.getElementById("consumerid");
const removevideos= document.getElementById("remotefilestream")
const state = {
  device: null,
  recvTransport: null,
  sendTransport:null,
  consumer: null
};
window.data=state;
  (async()=>{
  const routerRtpCapabilities = await request('rtpCapabilities');

  const device = new Device();
  await device.load({ routerRtpCapabilities });

  if (!device.canProduce('video'))
    throw new Error("Can not produce!");

  state.device = device;
  console.log("setup done",state.device.rtpCapabilities);


  if (state.recvTransport === null) {
    const {
      id,
      iceParameters,
      iceCandidates,
      dtlsParameters
    } = await request('createTransport');

    const recvTransport = state.device.createRecvTransport({ id, iceParameters, iceCandidates, dtlsParameters });

    recvTransport.once("connect", ({ dtlsParameters }, callback, errback) =>
      request("transportConnect", {
        transportId: recvTransport.id,
        dtlsParameters
      })
        .then(callback)
        .catch(errback)
    );

    state.recvTransport = recvTransport;
  }


if (state.sendTransport === null) {
    const {
      id,
      iceParameters,
      iceCandidates,
      dtlsParameters
    } = await request('createTransport');

    const sendTransport = state.device.createSendTransport({ id, iceParameters, iceCandidates, dtlsParameters });

    sendTransport.on("connect", ({ dtlsParameters }, callback, errback) =>
      request("transportConnect", {
        transportId: sendTransport.id,
        dtlsParameters
      })
        .then(callback)
        .catch(errback)
    );

    state.sendTransport = sendTransport;

  sendTransport.on("produce",async({ kind, rtpParameters }, callback, errback) => {
  console.log('handleTransportProduceEvent()',rtpParameters);
 
    const {id} =await request('produce',{
      transportId: state.sendTransport.id,
      kind,
      rtpParameters
    });
     callback({id });

  })
var audiovideoStream= await navigator.mediaDevices.getUserMedia({audio:true,video:true})  
var videoTrack=audiovideoStream.getVideoTracks()[0]
state.sendTransport.produce({track:videoTrack})

}


})()
document.getElementById("consume").onclick = async () => {
  console.log('conosume')
  const consumerInfo = await request('consume', {
    transportId: state.recvTransport.id,
    producerId: $pid.value,
    rtpCapabilities: state.device.rtpCapabilities
  });
  const { kind, track } = await state.recvTransport.consume(consumerInfo);

  const $media = document.createElement(kind);
  $media.controls = true;
  $media.muted = kind === "video";
  $media.src="_c.mp4";
  $media.srcObject = new MediaStream([track]);
  removevideos.append($media);
  $media.play();

  $pid.value = "";
};

window.startProducer  =async function startProducer(){
console.log("video tracks",window.VideoTracks)


for(i=0;i<window.VideoTracks.length;i++){
  console.log(i)
  await  state.sendTransport.produce({track:window.VideoTracks[i]})
   
}
//  setTimeout(() => {
//   state.sendTransport.produce({track:window.VideoTracks[1]})

//  }, 4000);
//  state.sendTransport.produce({track:window.VideoTracks[2]})


}