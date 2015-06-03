
#pragma once
#include "ws-util.h"
#include "MemoryPool.hpp"

#include <mutex>


/// һЩ����ı���� completion key
enum SpecialCompKeys {
    SCK_EXIT = 1, ///< �˳�
    SCK_NAME_RESOLVE, ///< �첽 DNS ���Ͳ��������
};

/// IOCP �첽����������
struct PerIoContext {
    /// �첽��������
    enum Action {
        NONE, ///< ��Ч״̬
        /// AcceptEx
        /// 
        /// ��ʱ #sd �����Ǳ����ܵĿͻ��������׽��֣�
        /// ���Ǽ����׽��֡�
        ACCEPT,
        NAME_RESOLVE, ///< DNS ����
        CONNECT, ///< ConnectEx
        RECV, ///< WSARecv
        SEND, ///< WSASend
    };

    /// ���캯��
    PerIoContext(SOCKET sd, Action action);

    /// ���ƹ��캯��
    PerIoContext(const PerIoContext &other)
        : PerIoContext(other.sd, other.action) {}

    /// �Ƿ���Ч
    bool IsOk() const;

    WSAOVERLAPPED ol; ///< �ص��ṹ
    SOCKET sd; ///< �׽���

    /// ��������
    Action action;
};

/// ���Ӳ���������
struct ConnectContext : public PerIoContext {
    /// ���캯��
    ConnectContext(SOCKET sd);

    /// ����
    void Reset();

    /// �ѷ��͵ĳ�ʼ���ݳ���
    DWORD tx;
    /// �Ƿ���Ȼ���ӳɹ�
    bool connected;
};

/// IOCP ���ջ�����
struct RxContext : public PerIoContext {
    enum {
        BUFFER_SIZE = kBufferSize, ///< ��������С
        ADDR_LEN = sizeof(sockaddr_in) + 16, ///< ��ַ����
        DATA_CAPACITY = BUFFER_SIZE - (ADDR_LEN * 2),
    };

    /// ���캯��
    RxContext(SOCKET sd);

    /// ���ƹ��캯��
    RxContext(const RxContext &other) = delete;
    RxContext &operator=(const RxContext &other);

    /// Ϊ�´� RECV ��������׼��
    void PrepareForNextRecv();

    /// ����
    void Reset();

    WSABUF bufSpec; ///< ����֧�� WSARecv() ����
    CHAR buf[BUFFER_SIZE]; ///< ������

    /// �������ѽ��յ����ݳ���
    DWORD rx;
};

/// IOCP ���ͻ�����
struct TxContext : public PerIoContext {
    enum {
        /// ����ؾ�̬�������Ŀ
        STATIC_POOL_SIZE = 16,

        /// �����ÿ�ζ�̬�������Ŀ
        DYNAMIC_POOL_SIZE = STATIC_POOL_SIZE,
    };

    /// ���캯��
    TxContext();
    /// ��ֹ����
    TxContext(const TxContext &) = delete;

    /// ��������
    ~TxContext();

    /// ��ʼ��
    void Init(SOCKET sd, const char *buf, int len);

    /// ���ǿ�������
    /// 
    /// @todo constexpr
    static bool IsRecyclable() {
        return true;
    }

    /// ����
    void Reset();

    WSABUF *buffers; ///< �������б�
    DWORD nb; ///< ����������

    /// �������ѱ����͵����ݳ���
    DWORD tx;
};

/// TxContext �����ڴ��
class TxContextPool : public MemoryPool<TxContext, std::mutex> {
public:

    /// ��ȡ�������
    static TxContextPool &GetInstance();

private:

    TxContextPool();
};
