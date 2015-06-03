
#pragma once
#include <list>

/// �յ��߳�ͬ������
struct NullThreadSynchronizer {
    /// ��������
    void lock() {}
    /// ��������
    void unlock() {}
};

/// �ڴ��
/// @param Node Ҫ����Ľ�����ͣ����붨����(��ʽ����ʾ��)��������
/// @param Synchronizer �̻߳���ͬ����
template <class Node, class Synchronizer>
class MemoryPool {
public:

    /// �б�����
    typedef std::list<Node *> List;

    /// ����Ԫ���������޷�����������
    typedef typename List::size_type size_type;

    /// ���캯��
    MemoryPool() {
        Init();
    }

    /// ��������
    ~MemoryPool() {
        Clear();
    }

    /// �������б��з���һ���½��
    Node *Allocate() {
        Node *ret = nullptr;
        m_sync.lock();

        // �����б�
        if (!m_free.empty() && m_free.front()->IsRecyclable()) {
            ret = m_free.front();
            m_free.pop_front();
        }
        // ��̬�ռ�
        else if (m_numStaticUsed < Node::STATIC_POOL_SIZE) {
            ret = m_static + m_numStaticUsed;
            m_numStaticUsed++;
        }
        // ��̬�ռ�
        else {
            if (m_nextAvailable == m_end) {
                NewChunk();
            }

            ret = m_nextAvailable;
            m_nextAvailable++;
        }

        m_sync.unlock();
        return ret;
    }

    /// �����������һ�����
    ///
    /// ע�⣺������øý���������������
    void DeAllocate(Node *node) {
        m_sync.lock();
        m_free.push_back(node);
        m_sync.unlock();
    }

    /// Clear the pool
    /// 
    /// This causes memory occupied by nodes allocated by the pool
    /// to be freed. Any nodes allocated from the pool will no longer
    /// be valid.
    void Clear() {
        while (m_header != (BlockHeader *) m_static) {
            byte *previousBegin = m_header->previousBegin;

            delete [] (Node *) m_header;

            m_header = (BlockHeader *) previousBegin;
        }

        Init();
    }

#ifdef _DEBUG
    /// ��ȡ�ѷ���Ľ����Ŀ
    ///
    /// ���ڵ���ģʽ�¶��塣
    size_type GetTotalAllocated() const {
        return m_totalAllocated;
    }
#endif

private:

    // ��ʼ��Ϊ�Ӿ�̬�ռ��з���
    void Init() {
        m_numStaticUsed = 0;

        m_header = (BlockHeader *) m_static;
        m_nextAvailable = nullptr;
        m_end = nullptr;

#ifdef _DEBUG
        m_totalAllocated = Node::STATIC_POOL_SIZE;
#endif
    }

    // ��ݶ���
    typedef char byte;

    // ����Ķ�̬�ռ�ͷ
    struct BlockHeader {
        byte *previousBegin;
        //-------------------------------------
        byte __PADDINGS[sizeof(Node) - sizeof(byte *)];
    };

    // �����µ��ڴ��
    void NewChunk() {
        Node *raw = new Node[Node::DYNAMIC_POOL_SIZE + 1];

        BlockHeader *header = (BlockHeader *) raw;
        header->previousBegin = (byte *) m_header;
        m_header = header;

        m_nextAvailable = raw + 1;
        m_end = raw + Node::DYNAMIC_POOL_SIZE + 1;

#ifdef _DEBUG
        m_totalAllocated += Node::DYNAMIC_POOL_SIZE + 1;
#endif
    }

private:

#ifdef _DEBUG
    size_type m_totalAllocated;
#endif

    Synchronizer m_sync;

    Node m_static[Node::STATIC_POOL_SIZE];

    // �ѷ���ľ�̬����Ԫ����Ŀ
    size_type m_numStaticUsed;

    // Start of raw memory making up current pool
    BlockHeader *m_header;
    // First free byte in current pool
    Node *m_nextAvailable;
    // One past last available byte in current pool
    Node *m_end;

    List m_free;
};
