#ifndef HILINK_TCP_BUNDLE_SINK_H
#define HILINK_TCP_BUNDLE_SINK_H 1

#include <cstdint>
#include <memory>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "PaddedVectorUint8.h"
#include "StcpBundleSink.h"
#include "TelemetryDefinitions.h"
#include "hilink_lib_export.h"

class HilinkTcpBundleSink {
public:
    typedef StcpBundleSink::WholeBundleReadyCallback_t WholeBundleReadyCallback_t;
    typedef StcpBundleSink::NotifyReadyToDeleteCallback_t NotifyReadyToDeleteCallback_t;

    HILINK_LIB_EXPORT HilinkTcpBundleSink(std::shared_ptr<boost::asio::ip::tcp::socket> tcpSocketPtr,
        boost::asio::io_service & tcpSocketIoServiceRef,
        uint8_t expectedHeaderByte,
        uint8_t expectedTrailerByte,
        const WholeBundleReadyCallback_t & wholeBundleReadyCallback,
        unsigned int numCircularBufferVectors,
        uint64_t maxBundleSizeBytes,
        const NotifyReadyToDeleteCallback_t & notifyReadyToDeleteCallback = NotifyReadyToDeleteCallback_t());
    HILINK_LIB_EXPORT ~HilinkTcpBundleSink();

    HILINK_LIB_EXPORT bool ReadyToBeDeleted();
    HILINK_LIB_EXPORT void GetTelemetry(StcpInductConnectionTelemetry_t& telem) const;

private:
    void HandleHilinkBundle(padded_vector_uint8_t & bundle);

    const uint8_t m_expectedHeaderByte;
    const uint8_t m_expectedTrailerByte;
    const WholeBundleReadyCallback_t m_wholeBundleReadyCallback;
    StcpBundleSink m_stcpBundleSink;
};

#endif // HILINK_TCP_BUNDLE_SINK_H
