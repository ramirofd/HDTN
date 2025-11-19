#ifndef HILINK_TCP_INDUCT_H
#define HILINK_TCP_INDUCT_H 1

#include <string>
#include "Induct.h"
#include "HilinkTcpBundleSink.h"
#include <list>
#include <memory>
#include <atomic>

class CLASS_VISIBILITY_INDUCT_MANAGER_LIB HilinkTcpInduct : public Induct {
public:
    INDUCT_MANAGER_LIB_EXPORT HilinkTcpInduct(const InductProcessBundleCallback_t & inductProcessBundleCallback,
        const induct_element_config_t & inductConfig, const uint64_t maxBundleSizeBytes,
        const OnNewOpportunisticLinkCallback_t& onNewOpportunisticLinkCallback,
        const OnDeletedOpportunisticLinkCallback_t& onDeletedOpportunisticLinkCallback);
    INDUCT_MANAGER_LIB_EXPORT virtual ~HilinkTcpInduct() override;
    INDUCT_MANAGER_LIB_EXPORT virtual void PopulateInductTelemetry(InductTelemetry_t& inductTelem) override;

private:
    INDUCT_MANAGER_LIB_EXPORT HilinkTcpInduct();
    INDUCT_MANAGER_LIB_EXPORT void StartTcpAccept();
    INDUCT_MANAGER_LIB_EXPORT void HandleTcpAccept(std::shared_ptr<boost::asio::ip::tcp::socket> & newTcpSocketPtr, const boost::system::error_code& error);
    INDUCT_MANAGER_LIB_EXPORT void ConnectionReadyToBeDeletedNotificationReceived();
    INDUCT_MANAGER_LIB_EXPORT void RemoveInactiveTcpConnections();
    INDUCT_MANAGER_LIB_EXPORT void DisableRemoveInactiveTcpConnections();

    boost::asio::io_service m_ioService;
    boost::asio::ip::tcp::acceptor m_tcpAcceptor;
    std::unique_ptr<boost::asio::io_service::work> m_workPtr;
    std::unique_ptr<boost::thread> m_ioServiceThreadPtr;
    std::list<HilinkTcpBundleSink> m_listHilinkTcpBundleSinks;
    boost::mutex m_listHilinkTcpBundleSinksMutex;
    std::atomic<bool> m_allowRemoveInactiveTcpConnections;
    const uint64_t M_MAX_BUNDLE_SIZE_BYTES;
};

#endif // HILINK_TCP_INDUCT_H
