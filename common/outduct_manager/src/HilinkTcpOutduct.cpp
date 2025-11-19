#include "HilinkTcpOutduct.h"
#include <boost/make_unique.hpp>
#include <memory>
#include <boost/lexical_cast.hpp>

HilinkTcpOutduct::HilinkTcpOutduct(const outduct_element_config_t & outductConfig, const uint64_t outductUuid) :
    Outduct(outductConfig, outductUuid),
    m_hilinkTcpBundleSource(outductConfig.hilinkHeaderByte, outductConfig.hilinkTrailerByte,
        outductConfig.keepAliveIntervalSeconds, outductConfig.maxNumberOfBundlesInPipeline + 5)
{}

HilinkTcpOutduct::~HilinkTcpOutduct() {}

std::size_t HilinkTcpOutduct::GetTotalBundlesUnacked() const noexcept {
    return m_hilinkTcpBundleSource.GetTotalBundlesUnacked();
}

bool HilinkTcpOutduct::Forward(const uint8_t* bundleData, const std::size_t size, std::vector<uint8_t>&& userData) {
    return m_hilinkTcpBundleSource.Forward(bundleData, size, std::move(userData));
}

bool HilinkTcpOutduct::Forward(zmq::message_t & movableDataZmq, std::vector<uint8_t>&& userData) {
    return m_hilinkTcpBundleSource.Forward(movableDataZmq, std::move(userData));
}

bool HilinkTcpOutduct::Forward(padded_vector_uint8_t& movableDataVec, std::vector<uint8_t>&& userData) {
    return m_hilinkTcpBundleSource.Forward(movableDataVec, std::move(userData));
}

void HilinkTcpOutduct::SetOnFailedBundleVecSendCallback(const OnFailedBundleVecSendCallback_t& callback) {
    m_hilinkTcpBundleSource.SetOnFailedBundleVecSendCallback(callback);
}
void HilinkTcpOutduct::SetOnFailedBundleZmqSendCallback(const OnFailedBundleZmqSendCallback_t& callback) {
    m_hilinkTcpBundleSource.SetOnFailedBundleZmqSendCallback(callback);
}
void HilinkTcpOutduct::SetOnSuccessfulBundleSendCallback(const OnSuccessfulBundleSendCallback_t& callback) {
    m_hilinkTcpBundleSource.SetOnSuccessfulBundleSendCallback(callback);
}
void HilinkTcpOutduct::SetOnOutductLinkStatusChangedCallback(const OnOutductLinkStatusChangedCallback_t& callback) {
    m_hilinkTcpBundleSource.SetOnOutductLinkStatusChangedCallback(callback);
}
void HilinkTcpOutduct::SetUserAssignedUuid(uint64_t userAssignedUuid) {
    m_hilinkTcpBundleSource.SetUserAssignedUuid(userAssignedUuid);
}

void HilinkTcpOutduct::Connect() {
    m_hilinkTcpBundleSource.Connect(m_outductConfig.remoteHostname, boost::lexical_cast<std::string>(m_outductConfig.remotePort));
}
bool HilinkTcpOutduct::ReadyToForward() {
    return m_hilinkTcpBundleSource.ReadyToForward();
}
void HilinkTcpOutduct::Stop() {
    m_hilinkTcpBundleSource.Stop();
}
void HilinkTcpOutduct::GetOutductFinalStats(OutductFinalStats & finalStats) {
    finalStats.m_convergenceLayer = m_outductConfig.convergenceLayer;
    finalStats.m_totalBundlesAcked = m_hilinkTcpBundleSource.GetTotalBundlesAcked();
    finalStats.m_totalBundlesSent = m_hilinkTcpBundleSource.GetTotalBundlesSent();
}
void HilinkTcpOutduct::PopulateOutductTelemetry(std::unique_ptr<OutductTelemetry_t>& outductTelem) {
    std::unique_ptr<StcpOutductTelemetry_t> t = boost::make_unique<StcpOutductTelemetry_t>();
    m_hilinkTcpBundleSource.GetTelemetry(*t);
    outductTelem = std::move(t);
    outductTelem->m_linkIsUpPerTimeSchedule = m_linkIsUpPerTimeSchedule;
}
