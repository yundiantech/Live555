/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2024 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H265 video file.
// Implementation

#include "h265_source.h"
#include "h265_media_subsession.h"
#include "H265VideoRTPSink.hh"
#include "H265VideoStreamFramer.hh"

H265ServerMediaSubsession* H265ServerMediaSubsession::createNew(UsageEnvironment& env, Boolean reuseFirstSource) 
{
  return new H265ServerMediaSubsession(env, reuseFirstSource);
}

H265ServerMediaSubsession::H265ServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource)
  : OnDemandServerMediaSubsession(env, reuseFirstSource),
    fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL) 
{

}

H265ServerMediaSubsession::~H265ServerMediaSubsession() 
{
  delete[] fAuxSDPLine;
}

void H265ServerMediaSubsession::inputFrame(VideoEncodedFramePtr videoFrame)
{
    if (videoFrame->getIsKeyFrame())
    {
        m_last_key_frame = videoFrame;  
    }

    std::unique_lock<std::mutex> lck(m_mutex);
// printf("%s:%d m_source_list.size()=%d \n", __FILE__, __LINE__, m_source_list.size());
    for (H265Source* source : m_source_list)
    {
        if (source->isFirstFrame() && m_last_key_frame != nullptr)
        {
            source->inputFrame(m_last_key_frame);
        }

        // if (videoFrame != m_last_key_frame)
        {
            source->inputFrame(videoFrame);
        }
    }
}

static void afterPlayingDummy(void* clientData)
{
  H265ServerMediaSubsession* subsess = (H265ServerMediaSubsession*)clientData;
  subsess->afterPlayingDummy1();
}

void H265ServerMediaSubsession::afterPlayingDummy1() 
{
  // Unschedule any pending 'checking' task:
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
  // Signal the event loop that we're done:
  setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) 
{
  H265ServerMediaSubsession* subsess = (H265ServerMediaSubsession*)clientData;
  subsess->checkForAuxSDPLine1();
}

void H265ServerMediaSubsession::checkForAuxSDPLine1() 
{
  nextTask() = NULL;

  char const* dasl;
  if (fAuxSDPLine != NULL) {
    // Signal the event loop that we're done:
    setDoneFlag();
  } else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
    fAuxSDPLine = strDup(dasl);
    fDummyRTPSink = NULL;

    // Signal the event loop that we're done:
    setDoneFlag();
  } else if (!fDoneFlag) {
    // try again after a brief delay:
    int uSecsToDelay = 100000; // 100 ms
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
			      (TaskFunc*)checkForAuxSDPLine, this);
  }
}

char const* H265ServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) 
{
  if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

  if (fDummyRTPSink == NULL) { // we're not already setting it up for another, concurrent stream
    // Note: For H265 video files, the 'config' information (used for several payload-format
    // specific parameters in the SDP description) isn't known until we start reading the file.
    // This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
    // and we need to start reading data from our file until this changes.
    fDummyRTPSink = rtpSink;

    // Start reading the file:
    fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

    // Check whether the sink's 'auxSDPLine()' is ready:
    checkForAuxSDPLine(this);
  }

  envir().taskScheduler().doEventLoop(&fDoneFlag);

  return fAuxSDPLine;
}

FramedSource* H265ServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) 
{
  estBitrate = 500; // kbps, estimate
printf("%s:%d create stream pointer=%d \n", __FILE__, __LINE__, this);
  // Create the video source:
  H265Source* source = H265Source::createNew(envir());
  
  if (source == NULL) return NULL;

  auto delete_callback = [=](void *pointer)
  {
    printf("%s:%d delete_callback pointer=%d \n", __FILE__, __LINE__, pointer);
    std::unique_lock<std::mutex> lck(m_mutex);
    m_source_list.remove((H265Source*)pointer);
    printf("%s:%d delete_callback pointer=%d m_source_list.size()=%d \n", __FILE__, __LINE__, pointer, m_source_list.size());
  };

  source->setDeleteCallBackFunc(delete_callback);

  std::unique_lock<std::mutex> lck(m_mutex);
  m_source_list.push_back(source);
 printf("%s:%d create stream source=%d m_source_list.size()=%d \n", __FILE__, __LINE__, source, m_source_list.size());
  // Create a framer for the Video Elementary Stream:
  return H265VideoStreamFramer::createNew(envir(), source);
}

RTPSink* H265ServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/) 
{
    OutPacketBuffer::maxSize = 200000;
    return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
