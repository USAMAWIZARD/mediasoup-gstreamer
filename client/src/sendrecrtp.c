/*
   "rtpbin name=rtpbin",
    "audiotestsrc is-live=true ! opusenc ! rtpopuspay pt=101 ssrc=1 ! rtpbin.send_rtp_sink_0",
    `udpsrc port=${MediasoupSrcPort} caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)102" ! rtph264depay ! decodebin  ! x264enc bitrate=20000 key-int-max=60 speed-preset=veryfast tune=zerolatency ! video/x-h264,profile=baseline ! rtph264pay pt=102 ssrc=2 ! rtpbin.send_rtp_sink_1`,
    `rtpbin.send_rtp_src_0  ! udpsink host=${host} port=${audioRtpPort}`,
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
#define udpVideoCaps "application/x-rtp,media=(string)video,clock-rate=(int)90000,payload=(int)101,encoding-name=(string)VP8,ssrc=(uint)376392232"
#define rtpBinCaps "application/x-rtp,media=(string)video,clock-rate=(int)90000,payload=(int)101,encoding-name=(string)VP8,ssrc=(uint)524240182"
static gint counter;

GstElement *pipeline;
GstElement *rtpbin;
GstElement *udpsrc,*depay,*queue,*rtpjitterbuffer,*vp8enc,*pay,*udpsink;
GstElement *videoconvert;
GstElement *sink,*vp8dec,*rtcpsink,*rtcpsrc;
  GstCaps *caps;
  GstPadLinkReturn lres;
  GstPad *srcpad, *sinkpad;


static GMainLoop *loop;
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

  g_print ("new payload on pad: %s\n", GST_PAD_NAME (new_pad));

  sinkpad = gst_element_get_static_pad (depay, "sink");
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
  long recivingPort=8000;
  int sendPort=1;

  /* build */
g_print("arg count %d",argc);
for(int i=0 ;i<argc;i++){
  g_print("arg no  %d  arg %s \n",i,argv[i]);
}
if(argc>3){
  g_print("adffffffffffffffff");
   recivingPort= strtol(argv[3], NULL, 10);
   g_print("%ld",recivingPort);
}
pipeline  = gst_pipeline_new ("rtprecandsend");


rtpbin = gst_element_factory_make ("rtpbin", "rtpbin");
  g_object_set (rtpbin, "latency", 200, NULL);


//g_object_set (G_OBJECT (rtpbin), "buffer-mode", "0", NULL);

//g_object_set (G_OBJECT (rtpbin), "sdes",rtpBinCaps, NULL);

udpsrc = gst_element_factory_make ("udpsrc", "udpsrc");
g_object_set (G_OBJECT (udpsrc), "port", recivingPort, NULL);

caps = gst_caps_from_string (udpVideoCaps);
g_object_set (udpsrc, "caps", caps, NULL);
//g_object_set (rtpbin, "caps", caps, NULL);


rtpjitterbuffer = gst_element_factory_make ("rtpjitterbuffer","rtpjitterbuffer");

g_object_set (G_OBJECT (rtpjitterbuffer), "latency", "50 ", NULL);

depay = gst_element_factory_make ("rtpvp8depay","rtpvp8depay");
vp8dec = gst_element_factory_make ("vp8dec", "vp8dec");
queue = gst_element_factory_make ("queue","queue");
videoconvert = gst_element_factory_make ("videoconvert","videoconvert");
sink = gst_element_factory_make ("xvimagesink", "xvimagesink");
g_object_set (G_OBJECT (sink), "sync", 0, NULL);

  rtcpsink = gst_element_factory_make ("udpsink", "rtcpsink");
g_object_set (rtcpsink, "port", 9009, "host", "127.0.0.1", NULL);
  /* no need for synchronisation or preroll on the RTCP sink */
  g_object_set (rtcpsink, "async", FALSE, "sync", FALSE, NULL);

vp8enc=gst_element_factory_make ("vp8enc","vp8enc");
pay =gst_element_factory_make ("rtpvp8pay","rtpvp8pay");
g_object_set (G_OBJECT (pay), "pt",101, NULL);
g_object_set (G_OBJECT (pay), "ssrc",2 , NULL);

 //g_object_set (G_OBJECT (pay), "picture-id-mode",1, NULL);

  rtcpsrc = gst_element_factory_make ("udpsrc", "rtcpsrc");
  g_assert (rtcpsrc);
  g_object_set (rtcpsrc, "port", 9003, NULL);

udpsink=gst_element_factory_make ("udpsink","udpsink");
g_object_set (G_OBJECT(udpsink), "host", "127.0.0.1", NULL);
g_object_set (G_OBJECT(udpsink), "port", 8008, NULL);

//then add all elements together
gst_bin_add_many (GST_BIN (pipeline), rtpbin,rtcpsink,udpsrc, depay, vp8dec,vp8enc,pay,queue, udpsink, NULL);

//link everythink after source
if(gst_element_link_many (udpsrc,depay, vp8dec,vp8enc,pay,udpsink, NULL))
g_print("element linked");


  // srcpad = gst_element_get_static_pad (udpsrc, "src");
  // sinkpad = gst_element_get_request_pad (rtpbin, "recv_rtp_sink_0");
  // lres = gst_pad_link (srcpad, sinkpad);
  // g_assert (lres == GST_PAD_LINK_OK);
  // gst_object_unref (srcpad);

  // srcpad = gst_element_get_static_pad (rtpbin, "send_rtp_sink_0");
  // sinkpad = gst_element_get_static_pad (pay, "src");
  // if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
  //   g_error ("Failed to link rtpbin to rtpsink");
  // gst_object_unref (srcpad);
  // gst_object_unref (sinkpad);



//   srcpad = gst_element_get_static_pad (udpsink, "sink");
//   sinkpad = gst_element_get_request_pad (rtpbin, "send_rtp_src_0");
//   lres = gst_pad_link (srcpad, sinkpad);
//   g_assert (lres == GST_PAD_LINK_OK);
//   gst_object_unref (srcpad);
//     gst_object_unref (sinkpad);


  
//   srcpad = gst_element_get_static_pad (rtcpsrc, "src");
//   sinkpad = gst_element_get_request_pad (rtpbin, "recv_rtcp_sink_0");
//   lres = gst_pad_link (srcpad, sinkpad);
//   g_assert (lres == GST_PAD_LINK_OK);
//   gst_object_unref (srcpad);
//   gst_object_unref (sinkpad);

// srcpad = gst_element_get_request_pad (rtpbin, "send_rtcp_src_0");
//   sinkpad = gst_element_get_static_pad (rtcpsink, "sink");
//   lres = gst_pad_link (srcpad, sinkpad);
//   g_assert (lres == GST_PAD_LINK_OK);
//   gst_object_unref (sinkpad);
/*
 * Connect to the pad-added signal for the rtpbin.  This allows us to link
 * the dynamic RTP source pad to the depayloader when it is created.
 */
//g_signal_connect (rtspsrc, "pad-added",(GCallback) pad_added_handler, pipeline);


g_signal_connect (rtpbin, "pad-added", G_CALLBACK (pad_added_cb), depay);
g_signal_connect (rtpbin, "on-ssrc-active", G_CALLBACK (on_ssrc_active_cb),
    depay);
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

/*
vp8enc=gst_element_factory_make ("vp8enc","vp8enc");
pay =gst_element_factory_make ("rtpvp8pay","rtpvp8pay");
udpsink=gst_element_factory_make ("udpsink","udpsink");
g_object_set (G_OBJECT(udpsink), "host", "127.0.0.1", NULL);
g_object_set (G_OBJECT(udpsink), "port", 4004, NULL);


*/
