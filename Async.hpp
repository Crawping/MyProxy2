
#pragma once
#include "ws-util.h"
#include <ws2tcpip.h>
#include <ctime>

/// �첽���� DNS
class AsyncResolver {
public:

    class Callback;

    /// ���캯��
    AsyncResolver(Callback *callback);

    /// ��������
    ~AsyncResolver();

    /// DNS ��������
    struct Request {
        const char *host; ///< Զ��������
        u_short port; ///< �˿ں�
        void *userData; ///< �û��Զ�����Ϣ
    };

    /// �ύ��������
    bool PostResolve(const Request &request);

    /// ȡ����������
    void Cancel();

public:

    /// DNS ���Ͳ�����������Ϣ
    struct QueryContext {
        OVERLAPPED ol;
        ADDRINFOEX *results;

        wchar_t *host;
        u_short port;

        ADDRINFOEX hints;
        wchar_t service[6];

        time_t ts; // �ύʱ��ʱ���

        AsyncResolver *resolver;
        void *userData;

        /// ���캯��
        QueryContext(AsyncResolver *resolver, const Request &request);
        ~QueryContext();
    };

    /// �ص���
    class Callback {
    public:

        /// �ص��麯��
        /// 
        /// @param context ��������Ӧ�ýӹ����ָ��
        virtual void OnQueryCompleted(QueryContext *context) = 0;
    };

    /// ���ûص�����
    void SetCallback(Callback *cb) {
        m_callback = cb;
    }

private:

    static void CALLBACK OnDnsResolved(DWORD, DWORD, LPWSAOVERLAPPED);

private:

    Callback *m_callback = nullptr;
    time_t m_ts = 0;
};
