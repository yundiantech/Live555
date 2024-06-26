#pragma once

#ifndef _FILE_SERVER_MEDIA_SUBSESSION_HH
#include "FileServerMediaSubsession.hh"
#endif

#include "adts_source.h"

class ADTSAudioServerMediaSubsession: public OnDemandServerMediaSubsession//FileServerMediaSubsession
{
public:
  static ADTSAudioServerMediaSubsession* createNew(UsageEnvironment& env, Boolean reuseFirstSource);

  ///输入音频数据
  void inputFrame(AACFramePtr audioFrame);

protected:
  ADTSAudioServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource); // called only by createNew();
  virtual ~ADTSAudioServerMediaSubsession();

protected: // redefined virtual functions
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

    std::mutex m_mutex;
    std::list<ADTSAudioSource*> m_source_list;
    
};
