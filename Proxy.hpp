
#pragma once
#include "ws-util.h"
#include "ThreadPool.hpp" // ������� "ws-util.h" ֮��

struct RxContext;
class Request;

/// �������
class MyProxy {
public:

    /// ���캯��
    MyProxy();

    /// ��������
    ~MyProxy();

    /// ������ѭ��
    bool Run(const char *addr, u_short port);

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

    SOCKET m_listener;
    HANDLE m_cp;

    ThreadPool m_threadPool;

    int m_requestPoolSize;
    int m_numActiveRequests;
};
