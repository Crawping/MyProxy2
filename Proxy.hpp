
#pragma once
#include "ws-util.h"

#include <vector>
#include <atomic>
#include <memory>

struct RxContext;
class Request;

/// �������
class MyProxy {
public:

    /// ���캯��
    MyProxy();

    /// ��������
    ~MyProxy();

    /// ��ʼ����
    bool Start(const char *addr, u_short port);

private:

    // ��ȡ IOCP ��� API �ĺ���ָ��
    static bool GetIocpFunctionPointers(SOCKET sd);

    // ��ʼ�������׽���
    bool SetUpListener(const char *addr, int port);

    // �ύ @a num �� AcceptEx() �첽����
    bool SpawnAcceptors(int num);

    bool PostAccept(RxContext &context);

    // ���������߳�
    bool SpawnThreads();

    // �߳���ں���
    static DWORD CALLBACK ProxyHandler(PVOID pv);

    // ����һ�����������
    void DoAccept(RxContext &context);

private:

    HANDLE m_cp = nullptr;
    SOCKET m_listener;

    typedef std::vector<std::shared_ptr<RxContext>> AcceptorVec;

    AcceptorVec m_acceptors;
    std::atomic_int m_numExitedThreads; // ���˳����߳���Ŀ
};
