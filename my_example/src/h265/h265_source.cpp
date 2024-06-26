#include "h265_source.h"
// #include "GroupsockHelper.hh"

H265Source* H265Source::createNew(UsageEnvironment& env,
				unsigned preferredFrameSize,
				unsigned playTimePerFrame) 
{
  H265Source* newSource = new H265Source(env, preferredFrameSize, playTimePerFrame);
  return newSource;
}

H265Source::H265Source(UsageEnvironment& env,
					   unsigned preferredFrameSize,
					   unsigned playTimePerFrame)
  : FramedSource(env),
    fPlayTimePerFrame(playTimePerFrame), fLastPlayTime(0)
{

}

H265Source::~H265Source() 
{
  printf("%s:%d point=%d \n", __FILE__, __LINE__, this);

  if (m_delete_callback_func)
  {
    m_delete_callback_func(this);
  }

  std::unique_lock<std::mutex> lck(m_mutex_frames);
  m_frames.clear();
  m_con_frames.notify_all();

  // printf("%s:%d pointer=%d \n", __FILE__, __LINE__, this);
}

void H265Source::inputFrame(VideoEncodedFramePtr videoFrame)
{
  if (m_is_stop)
  {
    return;
  }
// static FILE *fp = fopen("/opt/out2.265", "wb");
// fwrite(videoFrame->getBuffer(), 1, videoFrame->getSize(), fp);

    std::unique_lock<std::mutex> lck(m_mutex_frames);
    m_is_first_frame = false;
    m_frames.push_back(videoFrame);
    m_con_frames.notify_all();
    // printf("%s:%d m_frames.size()=%d \n", __FILE__, __LINE__, m_frames.size());
}

#if 1

void H265Source::doGetNextFrame() 
{

// if (m_index == 0)
// {
//   if (fp == NULL)
//   {
//     fp = fopen("/opt/out.265", "rb");
//   }
// usleep(1000000);
  
// fFrameSize = fread(fTo, 1, fMaxSize, fp);
  printf("%s:%d fMaxSize=%d fFrameSize=%d pointer=%d m_is_stop=%d m_frames.size()=%d \n", __FILE__, __LINE__, fMaxSize, fFrameSize, this, m_is_stop, m_frames.size());

//     // We don't know a specific play time duration for this data,
//     // so just record the current time as being the 'presentation time':
//     gettimeofday(&fPresentationTime, NULL);

//   // Because the file read was done from the event loop, we can call the
//   // 'after getting' function directly, without risk of infinite recursion:
//   FramedSource::afterGetting(this);
//   return;
// }

  int need_size = fMaxSize;
  // char m_buffer[150000] = {0};
  int m_buffer_size = 0;

do {
    std::unique_lock<std::mutex> lck(m_mutex_frames);

    // while (m_frames.empty() && !m_is_stop)
    // {
    //   // printf("%s:%d \n", __FILE__, __LINE__);
    //     m_con_frames.wait(lck);
    //     // printf("%s:%d \n", __FILE__, __LINE__);
    // }

    if (m_frames.empty() && !m_is_stop)
    {
      // printf("%s:%d \n", __FILE__, __LINE__);
        m_con_frames.wait_for(lck, std::chrono::milliseconds(100));
        
        break;
        
        // printf("%s:%d \n", __FILE__, __LINE__);
    }

    if (m_frames.empty() || m_is_stop)
    {
      // printf("%s:%d \n", __FILE__, __LINE__);
      m_frames.clear();
      return;
    }

    VideoEncodedFramePtr videoFrame = m_frames.front();
    // lck.unlock();

    int frame_size = videoFrame->getSize() - frame_buf_index;
    int size = std::min(need_size, frame_size);
    memcpy(fTo + m_buffer_size, videoFrame->getBuffer() + frame_buf_index, size);
    // memcpy(m_buffer+m_buffer_size, videoFrame->getBuffer() + frame_buf_index, size);
    m_buffer_size += size;
    need_size -= size;

    frame_buf_index = size + frame_buf_index;
// printf("%s:%d need_size=%d frame_size=%d m_frames.size()=%d frame_buf_index=%d\n", __FILE__, __LINE__, need_size, frame_size, m_frames.size(), frame_buf_index);
    if (frame_buf_index >= videoFrame->getSize())
    {
        frame_buf_index = 0;
    }

    if (frame_buf_index == 0)
    {
        m_frames.pop_front();
    }
break;
    if (need_size <= 0)
    {
        break;
    }

  }while(1);

  // printf("%s:%d m_buffer_size=%d  fMaxSize=%d point=%d \n", __FILE__, __LINE__, m_buffer_size, fMaxSize, this);
  // memcpy(fTo, m_buffer, m_buffer_size);
  fFrameSize = m_buffer_size;
// static FILE *fp = fopen("/opt/out.265", "wb");
// fwrite(m_buffer, 1, m_buffer_size, fp);

    // We don't know a specific play time duration for this data,
    // so just record the current time as being the 'presentation time':
    gettimeofday(&fPresentationTime, NULL);

    // Because the file read was done from the event loop, we can call the
    // 'after getting' function directly, without risk of infinite recursion:
    FramedSource::afterGetting(this);

}
#else
void H265Source::doGetNextFrame() 
{
  if (m_is_stop)
  {
    return;
  }
  // Try to read as many bytes as will fit in the buffer provided (or "fPreferredFrameSize" if less)
  // if (fLimitNumBytesToStream && fNumBytesToStream < (u_int64_t)fMaxSize) {
  //   fMaxSize = (unsigned)fNumBytesToStream;
  // }
  // if (fPreferredFrameSize > 0 && fPreferredFrameSize < fMaxSize) {
  //   fMaxSize = fPreferredFrameSize;
  // }

  // static FILE *fp = fopen("/opt/out.265", "rb");
  if (fp == NULL)
  {
    fp = fopen("/opt/out.265", "rb");
  }

  
fFrameSize = fread(fTo, 1, fMaxSize, fp);
  printf("%s:%d fMaxSize=%d fFrameSize=%d pointer=%d \n", __FILE__, __LINE__, fMaxSize, fFrameSize, this);
  if (fFrameSize == 0) 
  {
    fseek(fp, 0, SEEK_SET);
    // return;
  }

    // We don't know a specific play time duration for this data,
    // so just record the current time as being the 'presentation time':
    gettimeofday(&fPresentationTime, NULL);

  // Because the file read was done from the event loop, we can call the
  // 'after getting' function directly, without risk of infinite recursion:
  FramedSource::afterGetting(this);

}


#endif

void H265Source::doStopGettingFrames() 
{
  printf("%s:%d pointer=%d \n", __FILE__, __LINE__, this);
  m_is_stop = true;
  std::unique_lock<std::mutex> lck(m_mutex_frames);
  m_frames.clear();
  m_con_frames.notify_all();
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
  printf("%s:%d pointer=%d \n", __FILE__, __LINE__, this);
// #ifndef READ_FROM_FILES_SYNCHRONOUSLY
//   envir().taskScheduler().turnOffBackgroundReadHandling(fileno(fFid));
//   fHaveStartedReading = False;
// #endif
}
