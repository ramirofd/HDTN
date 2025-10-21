#include "HilinkTcpInduct.h"
#include "Logger.h"
#include <iostream>
#include <boost/make_unique.hpp>
#include <memory>
#include <boost/bind/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "ThreadNamer.h"

using boost::placeholders::_1;

static constexpr hdtn::Logger::SubProcess subprocess = hdtn::Logger::SubProcess::none;

HilinkTcpInduct::HilinkTcpInduct(const InductProcessBundleCallback_t & inductProcessBundleCallback,
    const induct_element_config_t & inductConfig, const uint64_t maxBundleSizeBytes,
    const OnNewOpportunisticLinkCallback_t& onNewOpportunisticLinkCallback,
    const OnDeletedOpportunisticLinkCallback_t& onDeletedOpportunisticLinkCallback) :
    Induct(inductProcessBundleCallback, inductConfig),
    m_tcpAcceptor(m_ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), inductConfig.boundPort)),
    m_workPtr(boost::make_unique<boost::asio::io_service::work>(m_ioService)),
    m_allowRemoveInactiveTcpConnections(true),
    M_MAX_BUNDLE_SIZE_BYTES(maxBundleSizeBytes)
{
    m_onNewOpportunisticLinkCallback = onNewOpportunisticLinkCallback;
    m_onDeletedOpportunisticLinkCallback = onDeletedOpportunisticLinkCallback;

    StartTcpAccept();
    m_ioServiceThreadPtr = boost::make_unique<boost::thread>(boost::bind(&boost::asio::io_service::run, &m_ioService));
    ThreadNamer::SetIoServiceThreadName(m_ioService, "ioServiceHilinkTcpInduct");
}

HilinkTcpInduct::~HilinkTcpInduct() {
    if (m_tcpAcceptor.is_open()) {
        try {
            m_tcpAcceptor.close();
        }
        catch (const boost::system::system_error & e) {
            LOG_ERROR(subprocess) << "Error closing TCP Acceptor in HilinkTcpInduct::~HilinkTcpInduct:  " << e.what();
        }
    }
    boost::asio::post(m_ioService, boost::bind(&HilinkTcpInduct::DisableRemoveInactiveTcpConnections, this));
    while (m_allowRemoveInactiveTcpConnections) {
        try {
            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        }
        catch (const boost::thread_resource_error&) {}
        catch (const boost::thread_interrupted&) {}
        catch (const boost::condition_error&) {}
        catch (const boost::lock_error&) {}
    }
    m_listStcpBundleSinks.clear();
    m_workPtr.reset();

    if (m_ioServiceThreadPtr) {
        try {
            m_ioServiceThreadPtr->join();
            m_ioServiceThreadPtr.reset();
        }
        catch (const boost::thread_resource_error&) {
            LOG_ERROR(subprocess) << "error stopping HilinkTcpInduct io_service";
        }
    }
}

void HilinkTcpInduct::StartTcpAccept() {
    LOG_INFO(subprocess) << "waiting for hilink tcp connections";
    std::shared_ptr<boost::asio::ip::tcp::socket> newTcpSocketPtr = std::make_shared<boost::asio::ip::tcp::socket>(m_ioService);

    m_tcpAcceptor.async_accept(*newTcpSocketPtr,
        boost::bind(&HilinkTcpInduct::HandleTcpAccept, this, newTcpSocketPtr, boost::asio::placeholders::error));
}

void HilinkTcpInduct::HandleTcpAccept(std::shared_ptr<boost::asio::ip::tcp::socket> & newTcpSocketPtr, const boost::system::error_code& error) {
    if (!error) {
        LOG_INFO(subprocess) << "hilink tcp connection: " << newTcpSocketPtr->remote_endpoint().address() << ":" << newTcpSocketPtr->remote_endpoint().port();
        {
            boost::mutex::scoped_lock lock(m_listStcpBundleSinksMutex);
            m_listStcpBundleSinks.emplace_back(newTcpSocketPtr, m_ioService,
                boost::bind(&HilinkTcpInduct::HandleHilinkBundle, this, _1),
                m_inductConfig.numRxCircularBufferElements,
                M_MAX_BUNDLE_SIZE_BYTES,
                boost::bind(&HilinkTcpInduct::ConnectionReadyToBeDeletedNotificationReceived, this));
        }
        if (m_onNewOpportunisticLinkCallback) {
            m_onNewOpportunisticLinkCallback(0, this, &m_listStcpBundleSinks.back());
        }

        StartTcpAccept();
    }
    else if (error != boost::asio::error::operation_aborted) {
        LOG_ERROR(subprocess) << "tcp accept error: " << error.message();
    }
}

void HilinkTcpInduct::HandleHilinkBundle(padded_vector_uint8_t & bundle) {
    if (bundle.empty()) {
        return;
    }

    if (bundle[0] == m_inductConfig.hilinkHeaderByte) {
        bundle.erase(bundle.begin());
        m_inductProcessBundleCallback(bundle);
    }
    else {
        LOG_ERROR(subprocess) << "unexpected hilink header byte " << static_cast<int>(bundle[0])
                              << " expected " << static_cast<int>(m_inductConfig.hilinkHeaderByte);
    }
}

void HilinkTcpInduct::RemoveInactiveTcpConnections() {
    const OnDeletedOpportunisticLinkCallback_t& callbackRef = m_onDeletedOpportunisticLinkCallback;
    if (m_allowRemoveInactiveTcpConnections.load(std::memory_order_acquire)) {
        boost::mutex::scoped_lock lock(m_listStcpBundleSinksMutex);
        m_listStcpBundleSinks.remove_if([&callbackRef, this](StcpBundleSink& sink) {
            if (sink.ReadyToBeDeleted()) {
                if (callbackRef) {
                    callbackRef(0, this, &sink);
                }
                return true;
            }
            else {
                return false;
            }
        });
    }
}

void HilinkTcpInduct::DisableRemoveInactiveTcpConnections() {
    m_allowRemoveInactiveTcpConnections = false;
}

void HilinkTcpInduct::ConnectionReadyToBeDeletedNotificationReceived() {
    boost::asio::post(m_ioService, boost::bind(&HilinkTcpInduct::RemoveInactiveTcpConnections, this));
}

void HilinkTcpInduct::PopulateInductTelemetry(InductTelemetry_t& inductTelem) {
    inductTelem.m_convergenceLayer = "hilink_tcp";
    inductTelem.m_listInductConnections.clear();
    {
        boost::mutex::scoped_lock lock(m_listStcpBundleSinksMutex);
        for (std::list<StcpBundleSink>::const_iterator it = m_listStcpBundleSinks.cbegin(); it != m_listStcpBundleSinks.cend(); ++it) {
            std::unique_ptr<StcpInductConnectionTelemetry_t> t = boost::make_unique<StcpInductConnectionTelemetry_t>();
            it->GetTelemetry(*t);
            inductTelem.m_listInductConnections.emplace_back(std::move(t));
        }
    }
    if (inductTelem.m_listInductConnections.empty()) {
        std::unique_ptr<StcpInductConnectionTelemetry_t> c = boost::make_unique<StcpInductConnectionTelemetry_t>();
        c->m_connectionName = "null";
        c->m_inputName = std::string("*:") + boost::lexical_cast<std::string>(m_tcpAcceptor.local_endpoint().port());
        inductTelem.m_listInductConnections.emplace_back(std::move(c));
    }
}
