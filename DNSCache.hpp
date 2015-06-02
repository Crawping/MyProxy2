
#pragma once
#include "ws-util.h"
#include <ws2tcpip.h> // for ADDRINFOEX

#include <string>
#include <map>

/// DNS ����
class DNSCache {
public:

    /// ��ĿʧЧʱ��
    /// 
    /// Ĭ��Ϊһ��Сʱ֮��û���κη�����Ŀ��ʧЧ��
    /// ÿ�η��ʶ������ʱ�����
    static double EXPIRATION;

    /// WinSock ������Ķ���
    typedef ADDRINFOEX AI;

    /// һ��������Ŀ
    struct Entry {
        /// ���캯��
        Entry(const AI *ai_other);

        /// ���ø��ƹ��캯��
        Entry(const Entry &other) = delete;

        /// ��������
        ~Entry();

        /// ��Ŀ�Ƿ���Ч
        bool IsOk() const;

        AI ai;
        bool home = false;
        time_t ts = 0; ///< ���뻺���ʱ��
    };

    /// ��������
    static const Entry *Resolve(const std::string &dname);

    /// ��������Ӧ�� IP ��ַ���뻺��
    static void Add(const std::string &dname, const AI &ai);

    /// ɾ��ʧЧ��Ŀ
    static bool Remove(const std::string &dname);

private:

    typedef std::map<std::string, Entry> Cache;
    static Cache ms_cache;
};
