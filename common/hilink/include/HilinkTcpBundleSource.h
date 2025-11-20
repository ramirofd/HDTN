#ifndef HILINK_TCP_BUNDLE_SOURCE_H
#define HILINK_TCP_BUNDLE_SOURCE_H 1

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <zmq.hpp>
#include "PaddedVectorUint8.h"
#include "StcpBundleSource.h"
#include "TelemetryDefinitions.h"
#include "hilink_lib_export.h"

class HilinkTcpBundleSource {
public:
    HILINK_LIB_EXPORT HilinkTcpBundleSource(uint8_t headerByte, uint8_t trailerByte,
        uint16_t keepAliveIntervalSeconds, unsigned int maxUnacked = 100);
    HILINK_LIB_EXPORT ~HilinkTcpBundleSource();

    HILINK_LIB_EXPORT bool Forward(const uint8_t* bundleData, std::size_t size, std::vector<uint8_t>&& userData);
    HILINK_LIB_EXPORT bool Forward(zmq::message_t & movableDataZmq, std::vector<uint8_t>&& userData);
    HILINK_LIB_EXPORT bool Forward(padded_vector_uint8_t& movableDataVec, std::vector<uint8_t>&& userData);

    HILINK_LIB_EXPORT void SetOnFailedBundleVecSendCallback(const OnFailedBundleVecSendCallback_t& callback);
    HILINK_LIB_EXPORT void SetOnFailedBundleZmqSendCallback(const OnFailedBundleZmqSendCallback_t& callback);
    HILINK_LIB_EXPORT void SetOnSuccessfulBundleSendCallback(const OnSuccessfulBundleSendCallback_t& callback);
    HILINK_LIB_EXPORT void SetOnOutductLinkStatusChangedCallback(const OnOutductLinkStatusChangedCallback_t& callback);
    HILINK_LIB_EXPORT void SetUserAssignedUuid(uint64_t userAssignedUuid);

    HILINK_LIB_EXPORT void Connect(const std::string & hostname, const std::string & port);
    HILINK_LIB_EXPORT bool ReadyToForward();
    HILINK_LIB_EXPORT void Stop();

    HILINK_LIB_EXPORT std::size_t GetTotalBundlesAcked() const noexcept;
    HILINK_LIB_EXPORT std::size_t GetTotalBundlesSent() const noexcept;
    HILINK_LIB_EXPORT std::size_t GetTotalBundlesUnacked() const noexcept;

    HILINK_LIB_EXPORT void GetTelemetry(StcpOutductTelemetry_t& telem) const;

private:
    bool ForwardVector(padded_vector_uint8_t& data, std::vector<uint8_t>&& userData);

    const uint8_t m_headerByte;
    const uint8_t m_trailerByte;
    StcpBundleSource m_stcpBundleSource;
};

#endif // HILINK_TCP_BUNDLE_SOURCE_H
