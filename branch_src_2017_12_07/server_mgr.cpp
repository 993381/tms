#include "fd.h"
#include "server_mgr.h"
#include "socket_util.h"
#include "tcp_socket.h"

using namespace socket_util;

ServerMgr::ServerMgr(Epoller* epoller)
    :
    epoller_(epoller)
{
}

ServerMgr::~ServerMgr()
{
}

ServerProtocol* ServerMgr::GetOrCreateProtocol(Fd& socket)
{
    int fd = socket.GetFd();
    if (fd_protocol_.count(fd) == 0)
    {   
        fd_protocol_[fd] = new ServerProtocol(epoller_, &socket);
    }   

    return fd_protocol_[fd];
}

int ServerMgr::HandleRead(IoBuffer& io_buffer, Fd& socket)
{
	ServerProtocol* server_protocol = GetOrCreateProtocol(socket);

    int ret = kClose;

    while ((ret = server_protocol->Parse(io_buffer)) == kSuccess)
    {   
    }   

    return ret;
}

int ServerMgr::HandleClose(IoBuffer& io_buffer, Fd& socket)
{
	ServerProtocol* server_protocol = GetOrCreateProtocol(socket);

    server_protocol->OnStop();

    delete server_protocol;
    fd_protocol_.erase(socket.GetFd());
}

int ServerMgr::HandleError(IoBuffer& io_buffer, Fd& socket)
{
	ServerProtocol* server_protocol = GetOrCreateProtocol(socket);

    server_protocol->OnStop();

    delete server_protocol;
    fd_protocol_.erase(socket.GetFd());
}

int ServerMgr::HandleConnected(Fd& socket)
{
	ServerProtocol* server_protocol = GetOrCreateProtocol(socket);

    server_protocol->OnConnected();
}

int ServerMgr::HandleAccept(Fd& socket)
{
    cout << LMSG << endl;
	ServerProtocol* server_protocol = GetOrCreateProtocol(socket);

    server_protocol->OnAccept();
}

int ServerMgr::HandleTimerInSecond(const uint64_t& now_in_ms, const uint32_t& interval, const uint64_t& count)
{
    for (const auto& kv : fd_protocol_)
    {
        kv.second->EveryNSecond(now_in_ms, interval, count);
    }
}

int ServerMgr::ConnectServer(const string& app, const string& stream, const string& ip, const uint16_t& port)
{
	int fd = CreateNonBlockTcpSocket();
    
    if (fd < 0)
    {   
        cout << LMSG << "ConnectServer ret:" << fd << endl;
        return -1; 
    }   
    
    int ret = ConnectHost(fd, ip, port);
    
    if (ret < 0 && errno != EINPROGRESS)
    {   
        cout << LMSG << "Connect ret:" << ret << endl;
        return -1; 
    }   
    
    Fd* socket = new TcpSocket(epoller_, fd, (SocketHandle*)this);
    
    ServerProtocol* server_dst = GetOrCreateProtocol(*socket);
    
    server_dst->SetPullServer();
    server_dst->SetApp(app);
    server_dst->SetStreamName(stream);
    
    if (errno == EINPROGRESS)
    {   
        server_dst->GetTcpSocket()->SetConnecting();
        server_dst->GetTcpSocket()->EnableWrite();
    }   
    else
    {   
        server_dst->GetTcpSocket()->SetConnected();
        server_dst->GetTcpSocket()->EnableRead();
    }   
    
    cout << LMSG << endl;	
}
