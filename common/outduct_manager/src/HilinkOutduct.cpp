#include "HilinkOutduct.h"
#include <boost/make_unique.hpp>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <cstring>

HilinkOutduct::HilinkOutduct(const outduct_element_config_t & outductConfig, const uint64_t outductUuid) :
    Outduct(outductConfig, outductUuid, false),
    m_udpBundleSource(outductConfig.maxNumberOfBundlesInPipeline + 5, outductConfig.rateLimitPrecisionMicroSec) {}

HilinkOutduct::~HilinkOutduct() {}

std::size_t HilinkOutduct::GetTotalBundlesUnacked() const noexcept {
    return m_udpBundleSource.GetTotalUdpPacketsUnacked();
}

bool HilinkOutduct::Forward(const uint8_t* bundleData, const std::size_t size, std::vector<uint8_t>&& userData) {
    padded_vector_uint8_t data(size + 1);
    data[0] = m_outductConfig.hilinkHeaderByte;
    if (size > 0) {
        std::memcpy(data.data() + 1, bundleData, size);
    }
    return m_udpBundleSource.Forward(data, std::move(userData));
}

bool HilinkOutduct::Forward(zmq::message_t & movableDataZmq, std::vector<uint8_t>&& userData) {
    zmq::message_t data(movableDataZmq.size() + 1);
    uint8_t* ptr = static_cast<uint8_t*>(data.data());
    ptr[0] = m_outductConfig.hilinkHeaderByte;
    if (movableDataZmq.size() > 0) {
        std::memcpy(ptr + 1, movableDataZmq.data(), movableDataZmq.size());
    }
    return m_udpBundleSource.Forward(data, std::move(userData));
}

bool HilinkOutduct::Forward(padded_vector_uint8_t& movableDataVec, std::vector<uint8_t>&& userData) {
    movableDataVec.insert(movableDataVec.begin(), m_outductConfig.hilinkHeaderByte);
    return m_udpBundleSource.Forward(movableDataVec, std::move(userData));
}

void HilinkOutduct::SetOnFailedBundleVecSendCallback(const OnFailedBundleVecSendCallback_t& callback) {
    m_udpBundleSource.SetOnFailedBundleVecSendCallback(callback);
}
void HilinkOutduct::SetOnFailedBundleZmqSendCallback(const OnFailedBundleZmqSendCallback_t& callback) {
    m_udpBundleSource.SetOnFailedBundleZmqSendCallback(callback);
}
void HilinkOutduct::SetOnSuccessfulBundleSendCallback(const OnSuccessfulBundleSendCallback_t& callback) {
    m_udpBundleSource.SetOnSuccessfulBundleSendCallback(callback);
}
void HilinkOutduct::SetOnOutductLinkStatusChangedCallback(const OnOutductLinkStatusChangedCallback_t& callback) {
    m_udpBundleSource.SetOnOutductLinkStatusChangedCallback(callback);
}
void HilinkOutduct::SetUserAssignedUuid(uint64_t userAssignedUuid) {
    m_udpBundleSource.SetUserAssignedUuid(userAssignedUuid);
}
void HilinkOutduct::SetRate(uint64_t maxSendRateBitsPerSecOrZeroToDisable) {
    m_udpBundleSource.UpdateRate(maxSendRateBitsPerSecOrZeroToDisable);
}

void HilinkOutduct::Connect() {
    m_udpBundleSource.Connect(m_outductConfig.remoteHostname, boost::lexical_cast<std::string>(m_outductConfig.remotePort));
}
bool HilinkOutduct::ReadyToForward() {
    return m_udpBundleSource.ReadyToForward();
}
void HilinkOutduct::Stop() {
    m_udpBundleSource.Stop();
}
void HilinkOutduct::GetOutductFinalStats(OutductFinalStats & finalStats) {
    finalStats.m_convergenceLayer = m_outductConfig.convergenceLayer;
    finalStats.m_totalBundlesAcked = m_udpBundleSource.GetTotalUdpPacketsAcked();
    finalStats.m_totalBundlesSent = m_udpBundleSource.GetTotalUdpPacketsSent();
}
void HilinkOutduct::PopulateOutductTelemetry(std::unique_ptr<OutductTelemetry_t>& outductTelem) {
    std::unique_ptr<UdpOutductTelemetry_t> t = boost::make_unique<UdpOutductTelemetry_t>();
    m_udpBundleSource.GetTelemetry(*t);
    outductTelem = std::move(t);
    outductTelem->m_linkIsUpPerTimeSchedule = m_linkIsUpPerTimeSchedule;
}
