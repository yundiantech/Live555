
#include "adts_media_subsession.h"
#include "adts_source.h"
#include "MPEG4GenericRTPSink.hh"

ADTSAudioServerMediaSubsession* ADTSAudioServerMediaSubsession::createNew(UsageEnvironment& env, Boolean reuseFirstSource, u_int8_t profile, u_int8_t sampling_frequency_index, u_int8_t channel_configuration) 
{
  return new ADTSAudioServerMediaSubsession(env, reuseFirstSource, profile, sampling_frequency_index, channel_configuration);
}

ADTSAudioServerMediaSubsession::ADTSAudioServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource, u_int8_t profile, u_int8_t sampling_frequency_index, u_int8_t channel_configuration)
  : OnDemandServerMediaSubsession(env, reuseFirstSource) 
{
    m_profile = profile;
    m_sampling_frequency_index = sampling_frequency_index;
    m_channel = channel_configuration;
}

ADTSAudioServerMediaSubsession::~ADTSAudioServerMediaSubsession() 
{

}

void ADTSAudioServerMediaSubsession::inputFrame(AACFramePtr audioFrame)
{
    // if (videoFrame->getIsKeyFrame())
    // {
    //     m_last_key_frame = videoFrame;  
    // }

    std::unique_lock<std::mutex> lck(m_mutex);
    for (ADTSAudioSource* source : m_source_list)
    {
        // if (source->isFirstFrame() && m_last_key_frame != nullptr)
        // {
        //     source->inputFrame(m_last_key_frame);
        // }

        // if (videoFrame != m_last_key_frame)
        {
            source->inputFrame(audioFrame);
        }
    }
}

FramedSource* ADTSAudioServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
{
  estBitrate = 96; // kbps, estimate

  printf("%s:%d \n", __FILE__, __LINE__);
  // Create the video source:
  ADTSAudioSource* source = ADTSAudioSource::createNew(envir(), m_profile, m_sampling_frequency_index, m_channel); 
  
  if (source == NULL) return NULL;

  auto delete_callback = [=](void *pointer)
  {
    printf("%s:%d delete stream pointer=%d \n", __FILE__, __LINE__, pointer);
    std::unique_lock<std::mutex> lck(m_mutex);
    m_source_list.remove((ADTSAudioSource*)pointer);
    printf("%s:%d delete stream pointer=%d m_source_list.size()=%d \n", __FILE__, __LINE__, pointer, m_source_list.size());
  };

  source->setDeleteCallBackFunc(delete_callback);

  std::unique_lock<std::mutex> lck(m_mutex);
  m_source_list.push_back(source);
 printf("%s:%d create stream source=%d m_source_list.size()=%d \n", __FILE__, __LINE__, source, m_source_list.size());

  return source;
}

RTPSink* ADTSAudioServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) 
{
  ADTSAudioSource* adtsSource = (ADTSAudioSource*)inputSource;
  return MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock,
					rtpPayloadTypeIfDynamic,
					adtsSource->samplingFrequency(),
					"audio", "AAC-hbr", adtsSource->configStr(),
					adtsSource->numChannels());
}
