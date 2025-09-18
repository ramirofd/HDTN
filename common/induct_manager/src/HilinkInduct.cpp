#include "HilinkInduct.h"
#include "Logger.h"
#include <iostream>
#include <boost/make_unique.hpp>
#include <memory>
#include <boost/bind/bind.hpp>
#include "ThreadNamer.h"

using boost::placeholders::_1;

static constexpr hdtn::Logger::SubProcess subprocess = hdtn::Logger::SubProcess::none;

HilinkInduct::HilinkInduct(const InductProcessBundleCallback_t & inductProcessBundleCallback, const induct_element_config_t & inductConfig) :
    Induct(inductProcessBundleCallback, inductConfig) {

    m_udpBundleSinkPtr = boost::make_unique<UdpBundleSink>(m_ioService, inductConfig.boundPort,
        boost::bind(&HilinkInduct::HandleHilinkBundle, this, _1),
        m_inductConfig.numRxCircularBufferElements,
        m_inductConfig.numRxCircularBufferBytesPerElement,
        boost::bind(&HilinkInduct::ConnectionReadyToBeDeletedNotificationReceived, this));

    m_ioServiceThreadPtr = boost::make_unique<boost::thread>(boost::bind(&boost::asio::io_service::run, &m_ioService));
    ThreadNamer::SetIoServiceThreadName(m_ioService, "ioServiceHilinkInduct");
}

HilinkInduct::~HilinkInduct() {
    m_udpBundleSinkPtr.reset();
    if (m_ioServiceThreadPtr) {
        try {
            m_ioServiceThreadPtr->join();
            m_ioServiceThreadPtr.reset();
        }
        catch (const boost::thread_resource_error&) {
            LOG_ERROR(subprocess) << "error stopping HilinkInduct io_service";
        }
    }
}

void HilinkInduct::HandleHilinkBundle(padded_vector_uint8_t & bundle) {
    if (!bundle.empty()) {
        if (bundle[0] == m_inductConfig.hilinkHeaderByte) {
            bundle.erase(bundle.begin()); // remove header byte
            m_inductProcessBundleCallback(bundle);
        } else {
            LOG_ERROR(subprocess) << "unexpected hilink header byte " << static_cast<int>(bundle[0])
                                  << " expected " << static_cast<int>(m_inductConfig.hilinkHeaderByte);
        }
    }
}

void HilinkInduct::RemoveInactiveConnection() {
    if (m_udpBundleSinkPtr && m_udpBundleSinkPtr->ReadyToBeDeleted()) {
        m_udpBundleSinkPtr.reset();
    }
}

void HilinkInduct::ConnectionReadyToBeDeletedNotificationReceived() {
    boost::asio::post(m_ioService, boost::bind(&HilinkInduct::RemoveInactiveConnection, this));
}

void HilinkInduct::PopulateInductTelemetry(InductTelemetry_t& inductTelem) {
    inductTelem.m_convergenceLayer = "hilink";
    inductTelem.m_listInductConnections.clear();
    std::unique_ptr<UdpInductConnectionTelemetry_t> t = boost::make_unique<UdpInductConnectionTelemetry_t>();
    m_udpBundleSinkPtr->GetTelemetry(*t);
    inductTelem.m_listInductConnections.emplace_back(std::move(t));
}
