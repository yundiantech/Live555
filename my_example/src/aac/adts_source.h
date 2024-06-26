#pragma once

#include <list>
#include <mutex>
#include <condition_variable>
#include <functional>

#include <FramedSource.hh>
#include "frame/AACFrame.h"

class ADTSAudioSource: public FramedSource //FramedFileSource 
{
public:
  static ADTSAudioSource* createNew(UsageEnvironment& env, u_int8_t profile, u_int8_t sampling_frequency_index, u_int8_t channel_configuration);

  ///输入音频数据
  void inputFrame(AACFramePtr audioFrame);

  void setDeleteCallBackFunc(std::function<void(void *pointer)> func){m_delete_callback_func = func;}

  unsigned samplingFrequency() const { return fSamplingFrequency; }
  unsigned numChannels() const { return fNumChannels; }
  char const* configStr() const { return fConfigStr; }  // returns the 'AudioSpecificConfig' for this stream (in ASCII form)

private:
  ADTSAudioSource(UsageEnvironment& env, u_int8_t profile,  u_int8_t sampling_frequency_index, u_int8_t channel_configuration);	// called only by createNew()

  virtual ~ADTSAudioSource();

private:
  // redefined virtual functions:
  virtual void doGetNextFrame();

private:
  unsigned fSamplingFrequency;
  unsigned fNumChannels;
  unsigned fuSecsPerFrame;
  char fConfigStr[5];

    bool m_is_stop = false;

    std::function<void(void *pointer)> m_delete_callback_func = nullptr;
    std::mutex m_mutex_frames;
    std::condition_variable m_con_frames;
    std::list<AACFramePtr> m_frames;

};
