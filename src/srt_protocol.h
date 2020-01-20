#ifndef __SRT_PROTOCOL_H__
#define __SRT_PROTOCOL_H__

#include <stdint.h>
#include <stddef.h>

#include <string>

#include "media_publisher.h"
#include "media_subscriber.h"
#include "ts_reader.h"

class IoLoop;
class Fd;
class IoBuffer;
class SrtSocket;

class SrtProtocol
    : public MediaPublisher
    , public MediaSubscriber
    , public SocketHandler
{
public:
    SrtProtocol(IoLoop* io_loop, Fd* socket);
    ~SrtProtocol();

	virtual int HandleRead(IoBuffer& io_buffer, Fd& socket);
	virtual int HandleClose(IoBuffer& io_buffer, Fd& socket) { return OnStop(); }
	virtual int HandleError(IoBuffer& io_buffer, Fd& socket) { return OnStop(); }
	virtual int HandleConnected(Fd& socket) { return OnConnected(); }

	int Parse(IoBuffer& io_buffer);
    int OnStop();
    int OnConnected() { return 0; }

    int EveryNSecond(const uint64_t& now_in_ms, const uint32_t& interval, const uint64_t& count);
    int EveryNMillSecond(const uint64_t& now_in_ms, const uint32_t& interval, const uint64_t& count) { return 0; }

    SrtSocket* GetSrtSocket()
    {
        return (SrtSocket*)socket_;
    }

	void SetMediaPublisher(MediaPublisher* media_publisher)
    {   
        media_publisher_ = media_publisher;
    }

    int SendData(const std::string& data);

    void OnFrame(const Payload& video_frame);
    void OnHeader(const Payload& header_frame);

private:
    void OpenDumpFile();
    void Dump(const uint8_t* data, const int& len);

private:
	IoLoop* io_loop_;
    Fd* socket_;
    MediaPublisher* media_publisher_;
    TsReader ts_reader_;
    bool    register_publisher_stream_;

    int dump_fd_;
};

#endif // __SRT_PROTOCOL_H__
