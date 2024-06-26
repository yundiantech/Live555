#pragma once

#include <string>
#include <stdint.h>
#include <functional>

#include <list>
#include <mutex>
#include <condition_variable>

#include "util/thread.h"
#include "frame/AACFrame.h"
#include "frame/VideoEncodedFrame.h"

#include "h265/h265_media_subsession.h"
#include "aac/adts_media_subsession.h"

class RtspStreamManager : public Thread
{
public:
    RtspStreamManager();
    virtual ~RtspStreamManager();

    bool startServer();

    ///输入视频数据
    void inputVideoFrame(VideoEncodedFramePtr videoFrame, const bool is_sub = false);

    ///输入音频数据
    void inputAudioFrame(AACFramePtr audioFrame);

    std::list<std::string> getUrls();
    std::string getUrl(const int index);

protected:
    void run();

private:
    // MediaSource* m_video_source = nullptr;
    // MediaSource* m_audio_source = nullptr;

    // MediaSession* m_session_main = nullptr;
    // RtspServer* m_rtsp_server = nullptr;

    H265ServerMediaSubsession *m_video_session = nullptr;
    ADTSAudioServerMediaSubsession *m_audio_session = nullptr;

    /// 子码流
    H265ServerMediaSubsession *m_video_session_sub = nullptr;
    ADTSAudioServerMediaSubsession *m_audio_session_sub = nullptr;

    bool serverFunc();
    bool m_is_startting = false;
    bool m_server_start = false;

};
