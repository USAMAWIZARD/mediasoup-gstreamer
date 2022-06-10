/*
   "rtpbin name=rtpbin",
    "audiotestsrc is-live=true ! opusenc ! rtpopuspay pt=101 ssrc=1 ! rtpbin.send_rtp_sink_0",
    `udpsrc port=${MediasoupSrcPort} caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)102" ! rtph264depay ! decodebin  ! x264enc bitrate=20000 key-int-max=60 speed-preset=veryfast tune=zerolatency ! video/x-h264,profile=baseline ! rtph264pay pt=102 ssrc=2 ! rtpbin.send_rtp_sink_1`,
    `rtpbin.send_rtp_src_0  ! udpsink host=${host} port=${audioReciveRTPPort}`,
    `rtpbin.send_rtcp_src_0 ! udpsink host=${host} port=${audioRtcpPort} sync=false async=false`,
    `rtpbin.send_rtp_src_1  ! udpsink host=${host} port=${videoRtpPort}`,
    `rtpbin.send_rtcp_src_1 ! udpsink host=${host} port=${videoRtcpPort} sync=false async=false`

play streamed server to server  for testing to use rtpbin for clear video 
gst-launch-1.0 -v udpsrc port=8000  caps="application/x-rtp,media=(string)video,clock-rate=(int)90000,payload=(int)101,encoding-name=(string)VP8,ssrc=(uint)526399105" ! rtpjitterbuffer latency=50 ! rtpvp8depay ! vp8dec   ! videoconvert ! queue ! xvimagesink sync=false
                                                                                                                                                                                                                                     gcc rtpp.c -o rtpp `pkg-config --cflags --libs gstreamer-1.0` 
sudo PROCESS_NAME="GStreamer" node server.js
     
gst-launch-1.0 -v udpsrc port=8000 caps=" some caps " ! rtpjitterbuffer latency=50 ! rtpvp8depay ! vp8dec ! identity ! vp8enc ! rtpvp8pay ! udpsink port=8001 host=127.0.0.1
      */

    
#include <gst/gst.h>
#include<stdio.h>
#include<stdlib.h>
#define udpVideoCaps "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)100"
#define udpAudioCaps "application/x-rtp, media=(string)audio, clock-rate=(int)48000, encoding-name=(string)OPUS, payload=(int)102"

#define h264caps "video/x-h264,profile=baseline"
static gint counter;

GstElement *pipeline;
GstElement *rtpbin,*fakesink,*rtpopusdepay,*opusdec,*opusenc,*rtpopuspay,*audioRTPsrc,*audioRTPsink,*audioRTCPsink;
GstElement *udpsrc,*idelem,*h264parse,*depay,*h264parse,*queue,*rtpjitterbuffer,*vp8enc,*pay,*udpsink;
GstElement *videoconvert;
GstElement *sink,*vp8dec,*rtcpsink,*rtcpsrc,*rtph264depay,*decodebin,*x264enc,*rtph264pay;
  GstCaps *caps,*enccaps;
  GstPadLinkReturn lres;
  GstPad *srcpad, *sinkpad, *pad;
static GstClockTime ptimestamp=(guint64)5000000000;
int a,b;
static GMainLoop *loop;

static GMainLoop *loop;
static GstPadProbeReturn display_data (GstPad          *pad,
              GstPadProbeInfo *apsInfo,
              gpointer         user_data)
{
  //apsInfo = gst_buffer_ref(apsInfo);  
//gst_buffer_is_writable (apsInfo);
 // apsInfo = gst_buffer_make_writable(apsInfo);
  //   int fps = 30;

     //ptimestamp += gst_util_uint64_scale_int (1, GST_SECOND, 40);
//GstBuffer *buffer;

//  buffer = GST_PAD_PROBE_INFO_BUFFER (info);
   // buffer = gst_buffer_make_writable (buffer);

   //   a=GST_BUFFER_PTS (apsInfo) ;
    //  b=GST_BUFFER_DTS (apsInfo) ;
   //  GST_BUFFER_PTS (apsInfo)=ptimestamp;
  //   GST_BUFFER_DTS (apsInfo)=ptimestamp;
  printf("%d %d ",a,b);
  //   GST_BUFFER_DURATION (apsInfo) = gst_util_uint64_scale_int (1, GST_SECOND, fps);
    
}

static GstPadProbeReturn
cb_have_data (GstPad          *pad,
              GstPadProbeInfo *info,
              gpointer         user_data)
{
  gint x, y;
  GstMapInfo map;
  guint16 *ptr, t;
  GstBuffer *buffer;
  buffer = GST_PAD_PROBE_INFO_BUFFER (info);

  buffer = gst_buffer_make_writable (buffer);


  /* Making a buffer writable can fail (for example if it
   * cannot be copied and is used more than once)
   */
  if (buffer == NULL)
    return GST_PAD_PROBE_OK;
   a=GST_BUFFER_PTS (buffer) ;
     b=GST_BUFFER_DTS (buffer) ;
     GST_BUFFER_PTS (buffer)+=ptimestamp;
     GST_BUFFER_DTS (buffer)+=ptimestamp;
 // printf("%d %d ",a,b);
 
  /* Mapping a buffer can fail (non-writable) */
  // if (gst_buffer_map (buffer, &map, GST_MAP_WRITE)) {
  //         exit(0);

  //   ptr = (guint16 *) map.data;
  //   /* invert data */
  //   for (y = 0; y < 288; y++) {
  //     for (x = 0; x < 384 / 2; x++) {
  //       t = ptr[384 - 1 - x];
  //       ptr[384 - 1 - x] = ptr[x];
  //       ptr[x] = t;
  //     }
  //     ptr += 384;
  //   }

  //   gst_buffer_unmap (buffer, &map);
  // }

 // GST_PAD_PROBE_INFO_DATA (info) = buffer;

  return GST_PAD_PROBE_OK;
}
static void cb_new_pad (GstElement *element, GstPad *pad, gpointer data)
{
  gchar *name;
  GstElement *other = data;

  name = gst_pad_get_name (pad);
  g_print ("A new pad %s was created for %s\n", name, gst_element_get_name(element));
  g_free (name);

  g_print ("element %s will be linked to %s\n",
           gst_element_get_name(element),
           gst_element_get_name(other));
  gst_element_link(element, other);
}


static void
print_source_stats (GObject * source)
{
  GstStructure *stats;
  gchar *str;

  g_return_if_fail (source != NULL);

  /* get the source stats */
  g_object_get (source, "stats", &stats, NULL);

  /* simply dump the stats structure */
  str = gst_structure_to_string (stats);
  g_print ("source stats: %s\n", str);

  gst_structure_free (stats);
  g_free (str);
}

/* will be called when rtpbin signals on-ssrc-active. It means that an RTCP
 * packet was received from another source. */
static void
on_ssrc_active_cb (GstElement * rtpbin, guint sessid, guint ssrc,
    GstElement * depay)
{
  GObject *session, *isrc, *osrc;

  g_print ("got RTCP from session %u, SSRC %u\n", sessid, ssrc);

  /* get the right session */
  g_signal_emit_by_name (rtpbin, "get-internal-session", sessid, &session);

  /* get the internal source (the SSRC allocated to us, the receiver */
  g_object_get (session, "internal-source", &isrc, NULL);
  print_source_stats (isrc);

  /* get the remote source that sent us RTCP */
  g_signal_emit_by_name (session, "get-source-by-ssrc", ssrc, &osrc);
  print_source_stats (osrc);
}

/* will be called when rtpbin has validated a payload that we can depayload */
static void
pad_added_cb (GstElement * rtpbin, GstPad * new_pad, GstElement * depay)
{
  GstPad *sinkpad;
  GstPadLinkReturn lres;

    GstCaps *caps;
    GstStructure *structure;
    GstPad *audiopad, *videopad;
    gchar padtype[10];
    caps =  gst_pad_get_current_caps(new_pad);
    g_assert (caps != NULL);
    structure = gst_caps_get_structure (caps, 0);
    g_stpcpy(padtype,gst_value_serialize(gst_structure_get_value(structure,"media")));
    printf("recived a %s RTP src\n",gst_value_serialize(gst_structure_get_value(structure,"media")));
    g_print ("new payload on pad: %s \n", GST_PAD_NAME (new_pad));

    if (g_strrstr (padtype, "video")){
      printf("recived video pad");
      sinkpad = gst_element_get_static_pad (rtph264depay, "sink");

    } 
    else if(g_strrstr (padtype,"audio")){
      printf("recived audio pad");
        sinkpad = gst_element_get_static_pad (rtpopusdepay, "sink");

    }

  
  g_assert (sinkpad);

  lres = gst_pad_link (new_pad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (sinkpad);
}


gint
main (gint   argc,
      gchar *argv[])
{

  /* init GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);
  long videoReciveRTPPort=5000;
  long videoSendRTPPort=4000;
  long videoSendRTCPPort=3000,audioReciveRTPPort=6005,audioSendRTPPort=6006,audioSendRTCPPort=6007;
  /* build */
for(int i=0;i<argc;i++){
  printf("  %d %s \n",i,argv[i]);
}


if(argc>=3){
  printf("setting send and recive ports");
   videoReciveRTPPort= strtol(argv[1], NULL, 10);
   videoSendRTPPort =  strtol(argv[2], NULL, 10);
   videoSendRTCPPort =  strtol(argv[3], NULL, 10);
   audioReciveRTPPort= strtol(argv[4], NULL, 10);
   audioSendRTPPort =  strtol(argv[5], NULL, 10);
   audioSendRTCPPort =  strtol(argv[6], NULL, 10);

}

pipeline  = gst_pipeline_new ("rtprecandsend");


rtpbin = gst_element_factory_make ("rtpbin", "rtpbin");
  //g_object_set (rtpbin, "latency", 200, NULL);


//g_object_set (G_OBJECT (rtpbin), "buffer-mode", "0", NULL);

//g_object_set (G_OBJECT (rtpbin), "sdes",rtpBinCaps, NULL);
//video
udpsrc = gst_element_factory_make ("udpsrc", "udpsrc");
idelem = gst_element_factory_make ("identity", "identity-elem");
g_object_set (G_OBJECT (udpsrc), "port", videoReciveRTPPort, NULL);

caps = gst_caps_from_string (udpVideoCaps);
g_object_set (udpsrc, "caps", caps, NULL);


//g_object_set (rtpbin, "caps", caps, NULL);


rtpjitterbuffer = gst_element_factory_make ("rtpjitterbuffer","rtpjitterbuffer");

g_object_set (G_OBJECT (rtpjitterbuffer), "latency", "50 ", NULL);

rtph264depay = gst_element_factory_make ("rtph264depay","rtph264depay");
decodebin = gst_element_factory_make ("decodebin", "decodebin");
queue = gst_element_factory_make ("queue","queue");
x264enc = gst_element_factory_make ("x264enc","x264enc");
//g_object_set (G_OBJECT (x264enc), "bitrate", 200000, NULL);
g_object_set (G_OBJECT (x264enc), "key-int-max", 60, NULL);
g_object_set (G_OBJECT (x264enc), "speed-preset", 3, NULL);
g_object_set (G_OBJECT (x264enc), "tune",4, NULL);


h264parse = gst_element_factory_make ("h264parse","h264parse");

  rtcpsink = gst_element_factory_make ("udpsink", "rtcpsink");
g_object_set (G_OBJECT(rtcpsink), "port", videoSendRTCPPort, "host", "192.168.0.105", NULL);
  /* no need for synchronisation or preroll on the RTCP sink */
  g_object_set (G_OBJECT(rtcpsink), "async", FALSE, "sync", FALSE, NULL);
videoconvert=gst_element_factory_make ("videoconvert","videoconvert");

h264parse=gst_element_factory_make ("h264parse","h264parse");
fakesink=gst_element_factory_make ("fakesink","fakesink");

rtph264pay =gst_element_factory_make ("rtph264pay","rtph264pay");
g_object_set (G_OBJECT (rtph264pay), "pt",102, NULL);
g_object_set (G_OBJECT (rtph264pay), "ssrc",2 , NULL);

 //g_object_set (G_OBJECT (pay), "picture-id-mode",1, NULL);
rtcpsrc = gst_element_factory_make ("udpsrc", "rtcpsrc");
g_assert (rtcpsrc);
g_object_set (G_OBJECT(rtcpsrc), "port", 9003, NULL);

udpsink=gst_element_factory_make ("udpsink","udpsink");
g_object_set (G_OBJECT(udpsink), "host", "192.168.0.105", NULL);
g_object_set (G_OBJECT(udpsink), "port", videoSendRTPPort, NULL);


//then add all elements together
gst_bin_add_many (GST_BIN (pipeline), rtpbin,udpsrc, rtph264depay, decodebin,queue,x264enc,rtph264pay, udpsink,rtcpsrc,rtcpsink,idelem,h264parse, NULL);

//link everythink after source
if(gst_element_link_many (rtph264depay, decodebin ,NULL))
g_print("video elements linked");
g_signal_connect (decodebin, "pad-added", G_CALLBACK (cb_new_pad),x264enc );

if(gst_element_link_many (x264enc,rtph264pay, udpsink ,NULL))
g_print("video element linked");

//audio
audioRTPsrc=gst_element_factory_make ("udpsrc","audioRTPsrc");
g_object_set (G_OBJECT (audioRTPsrc), "port", audioReciveRTPPort, NULL);
caps = gst_caps_from_string (udpAudioCaps);

g_object_set (audioRTPsrc, "caps", caps, NULL);
rtpopusdepay=gst_element_factory_make ("rtpopusdepay","rtpopusdepay");
opusdec=gst_element_factory_make ("opusdec","opusdec");
opusenc=gst_element_factory_make ("opusenc","opusenc");
rtpopuspay=gst_element_factory_make ("rtpopuspay","rtpopuspay");
g_object_set (G_OBJECT (rtpopuspay), "pt",101, NULL);
g_object_set (G_OBJECT (rtpopuspay), "ssrc",1 , NULL);
audioRTPsink =gst_element_factory_make("udpsink","audioRTPsink");
g_object_set (G_OBJECT(audioRTPsink), "port", audioSendRTPPort, "host", "192.168.0.105", NULL);
audioRTCPsink =gst_element_factory_make ("udpsink","audioRTCPsink");
g_object_set (G_OBJECT(audioRTCPsink), "port", audioSendRTCPPort, "host", "192.168.0.105", NULL);
g_object_set (G_OBJECT(audioRTCPsink), "async", FALSE, "sync", FALSE, NULL);

//link audio elements
 gst_bin_add_many (GST_BIN (pipeline),audioRTCPsink,audioRTPsink,audioRTPsrc,rtpopusdepay,opusdec,opusenc,rtpopuspay , NULL);

if(gst_element_link_many (rtpopusdepay,opusdec, opusenc,rtpopuspay ,audioRTPsink,NULL))
g_print("audio element linked");


  
 // linking video rMediasoupVideoSrcPorttp and rtcp
  srcpad = gst_element_get_static_pad (udpsrc, "src");
  sinkpad = gst_element_get_request_pad (rtpbin, "recv_rtp_sink_0");
  lres = gst_pad_link (srcpad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (srcpad);


  srcpad = gst_element_get_request_pad (rtpbin, "send_rtcp_src_0");
  sinkpad = gst_element_get_static_pad (rtcpsink, "sink");
  lres = gst_pad_link (srcpad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (sinkpad);

//link audio  rtp   and RTCP 

  srcpad = gst_element_get_static_pad (audioRTPsrc, "src");
  sinkpad = gst_element_get_request_pad (rtpbin, "recv_rtp_sink_1");
  lres = gst_pad_link (srcpad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (srcpad);


  srcpad = gst_element_get_request_pad (rtpbin, "send_rtcp_src_1");
  sinkpad = gst_element_get_static_pad (audioRTCPsink, "sink");
  lres = gst_pad_link (srcpad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (sinkpad);

//   g_object_set(G_OBJECT(idelem), "signal-handoffs", TRUE, NULL);   //change time
//   g_signal_connect(idelem, "handoff",  (GCallback) display_data  , pipeline);


 g_signal_connect (rtpbin, "pad-added", G_CALLBACK (pad_added_cb), NULL);

pad = gst_element_get_static_pad (rtph264pay, "src");
  gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER,
      (GstPadProbeCallback) cb_have_data , NULL, NULL);
  gst_object_unref (pad);


/* Set the pipeline to "playing" state*/
gst_element_set_state (pipeline, GST_STATE_PLAYING);

/* pad added handler */

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  g_print ("stopping receiver pipeline\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  gst_object_unref (pipeline);


  return 0;
}

