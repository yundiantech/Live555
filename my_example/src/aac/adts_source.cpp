#include "adts_source.h"
#include <GroupsockHelper.hh>

static unsigned const samplingFrequencyTable[16] = {
  96000, 88200, 64000, 48000,
  44100, 32000, 24000, 22050,
  16000, 12000, 11025, 8000,
  7350, 0, 0, 0
};

// ADTSAudioSource* ADTSAudioSource::createNew(UsageEnvironment& env, char const* fileName) 
// {
//   FILE* fid = NULL;
//   do {
//     fid = OpenInputFile(env, fileName);
//     if (fid == NULL) break;

//     // Now, having opened the input file, read the fixed header of the first frame,
//     // to get the audio stream's parameters:
//     unsigned char fixedHeader[4]; // it's actually 3.5 bytes long
//     if (fread(fixedHeader, 1, sizeof fixedHeader, fid) < sizeof fixedHeader) break;

//     // Check the 'syncword':
//     if (!(fixedHeader[0] == 0xFF && (fixedHeader[1]&0xF0) == 0xF0)) {
//       env.setResultMsg("Bad 'syncword' at start of ADTS file");
//       break;
//     }

//     // Get and check the 'profile':
//     u_int8_t profile = (fixedHeader[2]&0xC0)>>6; // 2 bits
//     if (profile == 3) {
//       env.setResultMsg("Bad (reserved) 'profile': 3 in first frame of ADTS file");
//       break;
//     }

//     // Get and check the 'sampling_frequency_index':
//     u_int8_t sampling_frequency_index = (fixedHeader[2]&0x3C)>>2; // 4 bits
//     if (samplingFrequencyTable[sampling_frequency_index] == 0) {
//       env.setResultMsg("Bad 'sampling_frequency_index' in first frame of ADTS file");
//       break;
//     }

//     // Get and check the 'channel_configuration':
//     u_int8_t channel_configuration
//       = ((fixedHeader[2]&0x01)<<2)|((fixedHeader[3]&0xC0)>>6); // 3 bits

//     // If we get here, the frame header was OK.
//     // Reset the fid to the beginning of the file:
// #ifndef _WIN32_WCE
//     rewind(fid);
// #else
//     SeekFile64(fid, SEEK_SET,0);
// #endif
// #ifdef DEBUG
//     fprintf(stderr, "Read first frame: profile %d, "
// 	    "sampling_frequency_index %d => samplingFrequency %d, "
// 	    "channel_configuration %d\n",
// 	    profile,
// 	    sampling_frequency_index, samplingFrequencyTable[sampling_frequency_index],
// 	    channel_configuration);
// #endif
//     return new ADTSAudioSource(env, fid, profile,
// 				   sampling_frequency_index, channel_configuration);
//   } while (0);

//   // An error occurred:
//   CloseInputFile(fid);
//   return NULL;
// }


ADTSAudioSource* ADTSAudioSource::createNew(UsageEnvironment& env, u_int8_t profile, u_int8_t sampling_frequency_index, u_int8_t channel_configuration) 
{
    return new ADTSAudioSource(env, profile, sampling_frequency_index, channel_configuration);
}


ADTSAudioSource::ADTSAudioSource(UsageEnvironment& env, u_int8_t profile, u_int8_t samplingFrequencyIndex, u_int8_t channelConfiguration)
  : FramedSource(env) 
{
  fSamplingFrequency = samplingFrequencyTable[samplingFrequencyIndex];
  fNumChannels = channelConfiguration == 0 ? 2 : channelConfiguration;
  fuSecsPerFrame = (1024/*samples-per-frame*/*1000000) / fSamplingFrequency/*samples-per-second*/;

  // Construct the 'AudioSpecificConfig', and from it, the corresponding ASCII string:
  unsigned char audioSpecificConfig[2];
  u_int8_t const audioObjectType = profile + 1;
  audioSpecificConfig[0] = (audioObjectType<<3) | (samplingFrequencyIndex>>1);
  audioSpecificConfig[1] = (samplingFrequencyIndex<<7) | (channelConfiguration<<3);
  sprintf(fConfigStr, "%02X%02X", audioSpecificConfig[0], audioSpecificConfig[1]);
}

ADTSAudioSource::~ADTSAudioSource() 
{
  printf("%s:%d point=%d \n", __FILE__, __LINE__, this);

  if (m_delete_callback_func)
  {
    m_delete_callback_func(this);
  }

  std::unique_lock<std::mutex> lck(m_mutex_frames);
  m_frames.clear();
  m_con_frames.notify_all();

  printf("%s:%d point=%d \n", __FILE__, __LINE__, this);
}

void ADTSAudioSource::inputFrame(AACFramePtr audioFrame)
{

// static FILE *fp = fopen("/opt/out2.aac", "wb");
// fwrite(audioFrame->getBuffer(), 1, audioFrame->getSize(), fp);

    std::unique_lock<std::mutex> lck(m_mutex_frames);
    m_frames.push_back(audioFrame);
    m_con_frames.notify_all();
    // printf("%s:%d m_frames.size()=%d \n", __FILE__, __LINE__, m_frames.size());
}

// Note: We should change the following to use asynchronous file reading, #####
// as we now do with ByteStreamFileSource. #####
// void ADTSAudioSource::doGetNextFrame() 
// {
//   // Begin by reading the 7-byte fixed_variable headers:
//   unsigned char headers[7];
//   if (fread(headers, 1, sizeof headers, fFid) < sizeof headers
//       || feof(fFid) || ferror(fFid)) {
//     // The input source has ended:
//     handleClosure();
//     return;
//   }

//   // Extract important fields from the headers:
//   Boolean protection_absent = headers[1]&0x01;
//   u_int16_t frame_length = ((headers[3]&0x03)<<11) | (headers[4]<<3) | ((headers[5]&0xE0)>>5);
// #ifdef DEBUG
//   u_int16_t syncword = (headers[0]<<4) | (headers[1]>>4);
//   fprintf(stderr, "Read frame: syncword 0x%x, protection_absent %d, frame_length %d\n", syncword, protection_absent, frame_length);
//   if (syncword != 0xFFF) fprintf(stderr, "WARNING: Bad syncword!\n");
// #endif
//   unsigned numBytesToRead = frame_length > sizeof headers ? frame_length - sizeof headers : 0;

//   // If there's a 'crc_check' field, skip it:
//   if (!protection_absent) 
//   {
//     SeekFile64(fFid, 2, SEEK_CUR);
//     numBytesToRead = numBytesToRead > 2 ? numBytesToRead - 2 : 0;
//   }

//   // Next, read the raw frame data into the buffer provided:
//   if (numBytesToRead > fMaxSize) 
//   {
//     fNumTruncatedBytes = numBytesToRead - fMaxSize;
//     numBytesToRead = fMaxSize;
//   }
//   int numBytesRead = fread(fTo, 1, numBytesToRead, fFid);
//   if (numBytesRead < 0) numBytesRead = 0;
//   fFrameSize = numBytesRead;
//   fNumTruncatedBytes += numBytesToRead - numBytesRead;

//   // Set the 'presentation time':
//   if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) 
//   {
//     // This is the first frame, so use the current time:
//     gettimeofday(&fPresentationTime, NULL);
//   } 
//   else 
//   {
//     // Increment by the play time of the previous frame:
//     unsigned uSeconds = fPresentationTime.tv_usec + fuSecsPerFrame;
//     fPresentationTime.tv_sec += uSeconds/1000000;
//     fPresentationTime.tv_usec = uSeconds%1000000;
//   }

//   fDurationInMicroseconds = fuSecsPerFrame;

//   // Switch to another task, and inform the reader that he has data:
//   nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)FramedSource::afterGetting, this);
// }

void ADTSAudioSource::doGetNextFrame() 
{
do {
    std::unique_lock<std::mutex> lck(m_mutex_frames);

    while (m_frames.empty() && !m_is_stop)
    {
      // printf("%s:%d \n", __FILE__, __LINE__);
        m_con_frames.wait(lck);
        // printf("%s:%d \n", __FILE__, __LINE__);
    }

    if (m_frames.empty() || m_is_stop)
    {
      // printf("%s:%d \n", __FILE__, __LINE__);
      m_frames.clear();
      break;
    }

    AACFramePtr audioFrame = m_frames.front();
    m_frames.pop_front();
    lck.unlock();


  // Begin by reading the 7-byte fixed_variable headers:
  // unsigned char headers[7];
  // if (fread(headers, 1, sizeof headers, fFid) < sizeof headers
  //     || feof(fFid) || ferror(fFid)) {
  //   // The input source has ended:
  //   handleClosure();
  //   return;
  // }

  
  int pos = 0;
  int adts_header_size = 7;
  uint8_t* headers = audioFrame->getBuffer();
  pos += adts_header_size;

  // Extract important fields from the headers:
  Boolean protection_absent = headers[1]&0x01;
  u_int16_t frame_length = ((headers[3]&0x03)<<11) | (headers[4]<<3) | ((headers[5]&0xE0)>>5);
  
  // if (frame_length <= 0)
  // {
  //     frame_length = audioFrame->getSize() - adts_header_size;
  // }
#if 0
  u_int16_t syncword = (headers[0]<<4) | (headers[1]>>4);
  fprintf(stderr, "Read frame: syncword 0x%x, protection_absent %d, frame_length %d\n", syncword, protection_absent, frame_length);
  if (syncword != 0xFFF) fprintf(stderr, "WARNING: Bad syncword!\n");
#endif
  unsigned numBytesToRead = frame_length > adts_header_size ? frame_length - adts_header_size : 0;
// printf("%s:%d numBytesToRead=%d size=%d frame_length=%d\n", __FILE__, __LINE__, numBytesToRead, audioFrame->getSize(), frame_length);
  // If there's a 'crc_check' field, skip it:
  if (!protection_absent) 
  {
    // SeekFile64(fFid, 2, SEEK_CUR);
    pos += 2;
    numBytesToRead = numBytesToRead > 2 ? numBytesToRead - 2 : 0;
  }

  // Next, read the raw frame data into the buffer provided:
  if (numBytesToRead > fMaxSize) 
  {
    fNumTruncatedBytes = numBytesToRead - fMaxSize;
    numBytesToRead = fMaxSize;
  }

  if (numBytesToRead > (audioFrame->getSize() - pos))
  {
      printf("%s:%d numBytesToRead=%d size=%d frame_length=%d pos=%d \n\n\n", __FILE__, __LINE__, numBytesToRead, audioFrame->getSize(), frame_length, pos);
      break;
  }

  // int numBytesRead = fread(fTo, 1, numBytesToRead, fFid);
  memcpy(fTo, audioFrame->getBuffer() + pos, numBytesToRead);

  // if (numBytesRead < 0) numBytesRead = 0;
  // fFrameSize = numBytesRead;
  // fNumTruncatedBytes += numBytesToRead - numBytesRead;

  fFrameSize = numBytesToRead;
  fNumTruncatedBytes = 0;

  // Set the 'presentation time':
  if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) 
  {
    // This is the first frame, so use the current time:
    gettimeofday(&fPresentationTime, NULL);
  } 
  else 
  {
    // Increment by the play time of the previous frame:
    unsigned uSeconds = fPresentationTime.tv_usec + fuSecsPerFrame;
    fPresentationTime.tv_sec += uSeconds/1000000;
    fPresentationTime.tv_usec = uSeconds%1000000;
  }

  fDurationInMicroseconds = fuSecsPerFrame;

  // Switch to another task, and inform the reader that he has data:
  nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)FramedSource::afterGetting, this);

  }while(0);
}
