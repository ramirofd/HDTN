#include "HilinkTcpBundleSink.h"
#include "Logger.h"
#include <boost/bind/bind.hpp>

using boost::placeholders::_1;

static constexpr hdtn::Logger::SubProcess subprocess = hdtn::Logger::SubProcess::none;

HilinkTcpBundleSink::HilinkTcpBundleSink(std::shared_ptr<boost::asio::ip::tcp::socket> tcpSocketPtr,
    boost::asio::io_service & tcpSocketIoServiceRef,
    uint8_t expectedHeaderByte,
    uint8_t expectedTrailerByte,
    const WholeBundleReadyCallback_t & wholeBundleReadyCallback,
    unsigned int numCircularBufferVectors,
    uint64_t maxBundleSizeBytes,
    const NotifyReadyToDeleteCallback_t & notifyReadyToDeleteCallback)
    : m_expectedHeaderByte(expectedHeaderByte),
    m_expectedTrailerByte(expectedTrailerByte),
    m_wholeBundleReadyCallback(wholeBundleReadyCallback),
    m_stcpBundleSink(tcpSocketPtr, tcpSocketIoServiceRef,
        boost::bind(&HilinkTcpBundleSink::HandleHilinkBundle, this, _1),
        numCircularBufferVectors,
        maxBundleSizeBytes,
        notifyReadyToDeleteCallback) {}

HilinkTcpBundleSink::~HilinkTcpBundleSink() {}

bool HilinkTcpBundleSink::ReadyToBeDeleted() {
    return m_stcpBundleSink.ReadyToBeDeleted();
}

void HilinkTcpBundleSink::GetTelemetry(StcpInductConnectionTelemetry_t& telem) const {
    m_stcpBundleSink.GetTelemetry(telem);
}

void HilinkTcpBundleSink::HandleHilinkBundle(padded_vector_uint8_t & bundle) {
    if (bundle.size() < 2) {
        LOG_ERROR(subprocess) << "received hilink tcp bundle smaller than header+trailer (size=" << bundle.size() << ")";
        return;
    }

    if (bundle.front() != m_expectedHeaderByte) {
        LOG_ERROR(subprocess) << "unexpected hilink header byte " << static_cast<int>(bundle.front())
                              << " expected " << static_cast<int>(m_expectedHeaderByte);
        return;
    }

    if (bundle.back() != m_expectedTrailerByte) {
        LOG_ERROR(subprocess) << "unexpected hilink trailer byte " << static_cast<int>(bundle.back())
                              << " expected " << static_cast<int>(m_expectedTrailerByte);
        return;
    }

    bundle.erase(bundle.begin());
    bundle.pop_back();
    if (m_wholeBundleReadyCallback) {
        m_wholeBundleReadyCallback(bundle);
    }
}
