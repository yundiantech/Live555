#pragma once

#include <list>
#include <mutex>
#include <condition_variable>
#include <functional>

#include <FramedSource.hh>
#include "frame/VideoEncodedFrame.h"

class H265Source: public FramedSource 
{
public:
  static H265Source* createNew(UsageEnvironment& env,
					 unsigned preferredFrameSize = 0,
					 unsigned playTimePerFrame = 0);
  // "preferredFrameSize" == 0 means 'no preference'
  // "playTimePerFrame" is in microseconds

  ///输入视频数据
  void inputFrame(VideoEncodedFramePtr videoFrame);

  bool isFirstFrame(){return m_is_first_frame;}
  
  void setDeleteCallBackFunc(std::function<void(void *pointer)> func){m_delete_callback_func = func;}

protected:
  H265Source(UsageEnvironment& env,
		       unsigned preferredFrameSize,
		       unsigned playTimePerFrame);
	// called only by createNew()

  virtual ~H265Source();

  // FILE *fp = NULL;

private:
  // redefined virtual functions:
  virtual void doGetNextFrame();
  virtual void doStopGettingFrames();

private:
    // unsigned fPreferredFrameSize;
    unsigned fPlayTimePerFrame;
    unsigned fLastPlayTime;

    bool m_is_first_frame = true;
    bool m_is_stop = false;

    int frame_buf_index = 0; //记录m_frames里的第一帧拷贝了多少数据

    std::function<void(void *pointer)> m_delete_callback_func = nullptr;
    std::mutex m_mutex_frames;
    std::condition_variable m_con_frames;
    std::list<VideoEncodedFramePtr> m_frames;

};
