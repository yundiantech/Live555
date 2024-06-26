#pragma once

#ifndef _FILE_SERVER_MEDIA_SUBSESSION_HH
#include "FileServerMediaSubsession.hh"
#endif

#include "adts_source.h"

class ADTSAudioServerMediaSubsession: public OnDemandServerMediaSubsession//FileServerMediaSubsession
{
public:
  static ADTSAudioServerMediaSubsession* createNew(UsageEnvironment& env, Boolean reuseFirstSource,
          u_int8_t profile = 2, u_int8_t sampling_frequency_index = 8, u_int8_t channel_configuration = 1);

  ///输入音频数据
  void inputFrame(AACFramePtr audioFrame);

protected:
  ADTSAudioServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource, u_int8_t profile = 2, u_int8_t sampling_frequency_index = 8, u_int8_t channel_configuration = 1); // called only by createNew();
  virtual ~ADTSAudioServerMediaSubsession();

protected: // redefined virtual functions
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

    std::mutex m_mutex;
    std::list<ADTSAudioSource*> m_source_list;

//    int profile = 2;  //AAC LC，MediaCodecInfo.CodecProfileLevel.AACObjectLC;
//    int freqIdx = 8;  //16K, 见后面注释avpriv_mpeg4audio_sample_rates中32000对应的数组下标，来自ffmpeg源码
//    /*int avpriv_mpeg4audio_sample_rates[] = {
//        96000, 88200, 64000, 48000, 44100, 32000,
//                24000, 22050, 16000, 12000, 11025, 8000, 7350
//    };
//    int chanCfg = 1;  //见后面注释channel_configuration，Stero双声道立体声
    u_int8_t m_profile = 2;
    u_int8_t m_sampling_frequency_index = 8;
    u_int8_t m_channel = 1;
    
};
