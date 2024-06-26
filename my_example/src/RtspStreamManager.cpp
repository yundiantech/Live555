#include "RtspStreamManager.h"

#include <iostream>
#include <liveMedia/liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

// #include "t31_config.h"
// #include "util/types.h"

void announceURL(RTSPServer* rtspServer, ServerMediaSession* sms) 
{
  if (rtspServer == NULL || sms == NULL) return; // sanity check

  UsageEnvironment& env = rtspServer->envir();

  env << "Play this stream using the URL ";
  if (weHaveAnIPv4Address(env)) {
    char* url = rtspServer->ipv4rtspURL(sms);
    env << "\"" << url << "\"";
    delete[] url;
    if (weHaveAnIPv6Address(env)) env << " or ";
  }
  if (weHaveAnIPv6Address(env)) {
    char* url = rtspServer->ipv6rtspURL(sms);
    env << "\"" << url << "\"";
    delete[] url;
  }
  env << "\n";
}

RtspStreamManager::RtspStreamManager()    
{
    
}

RtspStreamManager::~RtspStreamManager()
{

}

void RtspStreamManager::run()
{
    serverFunc();
}

bool RtspStreamManager::startServer()
{
    this->start();
    return true;
}

bool RtspStreamManager::serverFunc()
{
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

  // Create 'groupsocks' for RTP and RTCP:
  struct sockaddr_storage destinationAddress;
  destinationAddress.ss_family = AF_INET;
  ((struct sockaddr_in&)destinationAddress).sin_addr.s_addr = chooseRandomIPv4SSMAddress(*env);
  // Note: This is a multicast address.  If you wish instead to stream
  // using unicast, then you should use the "testOnDemandRTSPServer"
  // test program - not this test program - as a model.

  const unsigned short rtpPortNum = 18888;
  const unsigned short rtcpPortNum = rtpPortNum+1;
  const unsigned char ttl = 255;

  const Port rtpPort(rtpPortNum);
  const Port rtcpPort(rtcpPortNum);

  Groupsock rtpGroupsock(*env, destinationAddress, rtpPort, ttl);
  rtpGroupsock.multicastSendOnly(); // we're a SSM source
  Groupsock rtcpGroupsock(*env, destinationAddress, rtcpPort, ttl);
  rtcpGroupsock.multicastSendOnly(); // we're a SSM source

  // Create a 'H265 Video RTP' sink from the RTP 'groupsock':
  // OutPacketBuffer::maxSize = 100000;
  // videoSink = H265VideoRTPSink::createNew(*env, &rtpGroupsock, 96);

  // Create (and start) a 'RTCP instance' for this RTP sink:
  const unsigned estimatedSessionBandwidth = 500; // in kbps; for RTCP b/w share
  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen+1];
  gethostname((char*)CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0'; // just in case
  // RTCPInstance* rtcp
  // = RTCPInstance::createNew(*env, &rtcpGroupsock,
	// 		    estimatedSessionBandwidth, CNAME,
	// 		    videoSink, NULL /* we're a server */,
	// 		    True /* we're a SSM source */);
  // // Note: This starts RTCP running automatically

  RTSPServer* rtspServer = NULL;

  do{
    
    rtspServer = RTSPServer::createNew(*env, 554);

    if (rtspServer == NULL) 
    {
      *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
      // AppConfig::mSleep(5000);
      usleep(5000000);
    }


  }while (rtspServer == NULL);

// rtspServer.setAuthenticationDatabase()

  // char const* descriptionString = "Session streamed by \"testOnDemandRTSPServer\"";

  // A H.265 video elementary stream:
  {
    // To make the second and subsequent client for each stream reuse the same
    // input stream as the first client (rather than playing the file from the
    // start for each client), change the following "False" to "True":
    Boolean reuseFirstSource = true;

    {
      m_video_session = H265ServerMediaSubsession::createNew(*env, reuseFirstSource);
      m_audio_session = ADTSAudioServerMediaSubsession::createNew(*env, reuseFirstSource);
printf("%s:%d m_video_session=%d \n", __FILE__, __LINE__, m_video_session);
      char const* streamName = "live001";
      ServerMediaSession* sms = ServerMediaSession::createNew(*env, streamName, streamName);
      sms->addSubsession(m_video_session);
      sms->addSubsession(m_audio_session);
      rtspServer->addServerMediaSession(sms);
      announceURL(rtspServer, sms);
    }
 
//     {
//       m_video_session_sub = H265ServerMediaSubsession::createNew(*env, reuseFirstSource);
//       m_audio_session_sub = ADTSAudioServerMediaSubsession::createNew(*env, reuseFirstSource);
// printf("%s:%d m_video_session_sub=%d \n", __FILE__, __LINE__, m_video_session_sub);
//       char const* streamName = "live002";
//       ServerMediaSession* sms = ServerMediaSession::createNew(*env, streamName, streamName);
//       sms->addSubsession(m_video_session_sub);
//       sms->addSubsession(m_audio_session_sub);
//       rtspServer->addServerMediaSession(sms);
//       announceURL(rtspServer, sms);
//     }

    
  }

  // Start the streaming:
  *env << "Beginning streaming...\n";
  // play();

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

std::string RtspStreamManager::getUrl(int index)
{
    std::string url;

    // if (m_rtsp_server)
    // {
    //     url = m_rtsp_server->getUrl(m_session_main);
    // }
    
    return url;
}

std::list<std::string> RtspStreamManager::getUrls()
{
    std::list<std::string> url_list;

    url_list.push_back(getUrl(0));
    
    return url_list;
}

void RtspStreamManager::inputVideoFrame(VideoEncodedFramePtr videoFrame, const bool is_sub)
{
    if (m_video_session && videoFrame && videoFrame.get())
    {
        if (!is_sub)
        {
            m_video_session->inputFrame(videoFrame);
        }
        // else if (m_video_session_sub)
        // {
        //     m_video_session_sub->inputFrame(videoFrame);
        // }
    }
}

void RtspStreamManager::inputAudioFrame(AACFramePtr audioFrame)
{
    if (m_audio_session && audioFrame && audioFrame.get())
    {
        m_audio_session->inputFrame(audioFrame);

        // if (m_audio_session_sub)
        // {
        //     m_audio_session_sub->inputFrame(audioFrame);
        // }
    }
}
