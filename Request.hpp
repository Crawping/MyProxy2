
#pragma once

#include "Logger.hpp"
#include "PerIoContext.hpp"
#include "Async.hpp"
#include "MemoryPool.hpp"
#include "ws-util.h"

#include <ctime>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <mutex>
using namespace std;

#define SOCKET_bind ::bind

/// �����������һ������
/// 
/// ���ܰ������� HTTP ��������
class Request : public AsyncResolver::Callback {
public:

    enum {
        /// ����ؾ�̬�������Ŀ
        STATIC_POOL_SIZE = 256,

        /// �����ÿ�ζ�̬�������Ŀ
        DYNAMIC_POOL_SIZE = STATIC_POOL_SIZE,
    };

    /// ���캯��
    Request();

    /// ��������
    ~Request();

    /// ��ʼ��
    void Init(HANDLE cp, const RxContext &acceptContext);

    /// �������������������
    void HandleBrowser();

    /// ��ݶ���
    typedef AsyncResolver::QueryContext QueryContext;

    /// �� IOCP �ַ��� DNS ����������¼�
    void OnIocpQueryCompleted(QueryContext &context);

    /// �첽���Ӳ��������
    void OnConnectCompleted();

    /// �첽�����������
    void OnRecvCompleted(RxContext &context);

    /// �첽д���������
    void OnSendCompleted(TxContext *&context);

public:

    /// ��ӡ HTTP ͷ�ĵ�һ�У����� GET��POST ����Ϣ
    void PrintRequest(Logger::OutputLevel level) const;

    /// ͳ����Ϣ
    struct Statistics {
        /// �ܵ�������
        atomic_int requests;

        /// ���롢����ֽ���
        atomic_llong inBytes, outBytes;

        /// DNS ��ѯ��
        atomic_int dnsQueries;

        /// DNS ����������
        atomic_int dnsCacheHit;
    };

    /// ��ȡͳ����Ϣ
    static Statistics GetStatistics();

public:

    /// ��ǰ�Ƿ��������
    bool IsRecyclable() const;

private:

    // ���ö���Ϊ����ࡱ��״̬
    void Clear();

    // �����Լ�
    void DeleteThis();

private:

    // ����������
    typedef vector<char> Buffer;

    // ���Դ������������
    bool TryParsingHeaders();

    // ��������
    void SplitHost(const string &decl, int defaultPort);

    // ������˵����������ϴ������
    void OnUploadDone();

    // ת����������
    bool HandleServer();
    bool DoHandleServer();

    // �Ͽ��������������
    void ShutdownBrowserSocket();

    // �Ͽ��������������
    bool ShutdownServerSocket();

    // ����ʹ�� DNS ����� IP ��ַ���ӵ�������
    bool TryDNSCache();

    // DNS �������
    virtual void OnQueryCompleted(QueryContext *context) override;

    // �ύ�첽 DNS ��������
    bool PostDnsQuery();

    // ���ٵ�ǰʹ�õ� QueryContext
    void DelQueryContext();

    // ʹ�õ�ַ�����е�ͷ������ӵ�������
    void PostConnect();

    // ��������������� HTTP ͷ��
    // 
    // ��Ҫ��ȥ������������Ϣ��
    void FilterBrowserHeaders();

    // �Ƿ���Ȼת������������������ݵ�������
    bool IsUploadDone() const;

    // �Ƿ�Ϊ����������� per-io-context
    bool IsBrowserOrientedContext(const PerIoContext &context);

    // �������������ٴ���һ�� TxContext
    static TxContext *NewTxContext(SOCKET sd, const RxContext &context);
    static TxContext *NewTxContext(SOCKET sd, const char *buf, int len);

    // ����һ�� TxContext
    static void DelTxContext(TxContext *context);

    // �ύ�첽��������
    bool PostRecv(RxContext &context);

    // �ύ�첽��������
    bool PostSend(TxContext *context);

    // ����/ȡ�����첽�����������ύ����־��������
    void SetRxReqPostedMark(bool browser, bool posted);

private:

    void LogInfo(const string &msg) const;
    void LogError(const string &msg) const;

    void Log(const string &msg, Logger::OutputLevel level) const;

private:

    HANDLE m_cp = nullptr;

    // ����������������İ������� HTTP ͷ����һ������
    // ���ܲ�����ֻ�� HTTP ͷ����Ϣ��
    Buffer m_vbuf;

    struct Host {
        void Clear() {
            this->name.clear();
            this->port = 0;
            this->tunel = false;
        }

        bool operator!=(const Host &other) {
            return this->name != other.name || this->port != other.port;
        }

        // ��ȡȫ�������϶˿ڣ�
        string GetFullName() const;

        string name;
        unsigned short port = 0;

        bool tunel = false; // �Ƿ�Ϊ���
    };

    Host m_host;

    AsyncResolver m_resolver;
    QueryContext *m_qcontext = nullptr;
    ADDRINFOEX *m_ai = nullptr; // ��ǰ���Ե� addrinfo �ṹ
    ConnectContext m_ccontext;

    // HTTP ͷ��
    struct Headers {
    public:

        // ����ͷ��
        // 
        // @param buf ���뱣֤�� 0 ��β
        // @param browser �Ƿ����������
        bool Parse(const char *buf, bool browser);

        // �Ƿ��Ѿ������ɹ�
        bool IsOk() const;

        // �������
        void Clear();

        // �Ƿ񱣳�����
        bool KeepAlive() const;

        // ����״̬��ȷ�������Ƿ���Ȼ����
        bool DetermineFinishedByStatusCode() const;

        // �Ƿ�ֶ�
        bool IsChunked() const;

    public:

        int status_code = 0;

        unordered_map<string, string> m;
        int bodyOffset = -1;
    };

    Headers m_headers;

    RxContext m_bcontext;
    size_t m_btotal = 0; // ��ǰ����ȫ����
    size_t m_brx = 0; // ��ǰ�����ѽ��յ����ݳ���

    RxContext m_scontext;

    bool m_brxPosted = false; // ��ǰ�Ƿ��������������Ľ�������
    bool m_srxPosted = false; // ��ǰ�Ƿ��������������Ľ�������

    // ͳ����Ϣ
    static Statistics ms_stat;

private:

    time_t m_delTS = 0; // ��ɾ��ʱ��ʱ���

private:

    // ������
    bool m_everRx = false; // ȷʵ���յ�������
    bool m_noAttachedData = true; // һ��ʼ��û�����ݽ���

    // ���Է������ĵ�һ�������Ƿ���Ȼ�յ�
    bool m_firstResponseRecv = false;
};

/// Request �����ڴ��
class RequestPool : public MemoryPool<Request, mutex> {
public:

    /// ��ȡ�������
    static RequestPool &GetInstance();

private:

    RequestPool() {}
};
