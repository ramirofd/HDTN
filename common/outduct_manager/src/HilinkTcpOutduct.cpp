#include "HilinkTcpOutduct.h"
#include <boost/make_unique.hpp>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <cstring>

HilinkTcpOutduct::HilinkTcpOutduct(const outduct_element_config_t & outductConfig, const uint64_t outductUuid) :
    Outduct(outductConfig, outductUuid),
    m_stcpBundleSource(outductConfig.keepAliveIntervalSeconds, outductConfig.maxNumberOfBundlesInPipeline + 5)
{}

HilinkTcpOutduct::~HilinkTcpOutduct() {}

std::size_t HilinkTcpOutduct::GetTotalBundlesUnacked() const noexcept {
    return m_stcpBundleSource.GetTotalBundlesUnacked();
}

bool HilinkTcpOutduct::Forward(const uint8_t* bundleData, const std::size_t size, std::vector<uint8_t>&& userData) {
    // [1B header][4B length BE][payload][1B trailer]
    const std::size_t encodedSize = 1 + 4 + size + 1;
    padded_vector_uint8_t data(encodedSize);

    data[0] = m_outductConfig.hilinkHeaderByte;

    uint32_t len = static_cast<uint32_t>(size);
    data[1] = static_cast<uint8_t>((len >> 24) & 0xFF);
    data[2] = static_cast<uint8_t>((len >> 16) & 0xFF);
    data[3] = static_cast<uint8_t>((len >> 8)  & 0xFF);
    data[4] = static_cast<uint8_t>(len & 0xFF);

    if (size > 0) {
        std::memcpy(data.data() + 5, bundleData, size);
    }
    data[encodedSize - 1] = m_outductConfig.hilinkTrailerByte;
    return m_stcpBundleSource.Forward(data, std::move(userData));

}

bool HilinkTcpOutduct::Forward(zmq::message_t & movableDataZmq, std::vector<uint8_t>&& userData) {
    // [1B header][4B length BE][payload][1B trailer]
    const uint32_t len = static_cast<uint32_t>(movableDataZmq.size());
    zmq::message_t data(1 + 4 + len + 1);
    uint8_t* p = static_cast<uint8_t*>(data.data());
    p[0] = m_outductConfig.hilinkHeaderByte;
    p[1] = (len >> 24) & 0xFF;
    p[2] = (len >> 16) & 0xFF;
    p[3] = (len >> 8)  & 0xFF;
    p[4] = len & 0xFF;
    std::memcpy(p + 5, movableDataZmq.data(), len);
    p[5 + len] = m_outductConfig.hilinkTrailerByte;
    return m_stcpBundleSource.Forward(data, std::move(userData));
}

bool HilinkTcpOutduct::Forward(padded_vector_uint8_t& movableDataVec, std::vector<uint8_t>&& userData) {
    movableDataVec.insert(movableDataVec.begin(), m_outductConfig.hilinkHeaderByte);
    movableDataVec.push_back(m_outductConfig.hilinkTrailerByte);
    return m_stcpBundleSource.Forward(movableDataVec, std::move(userData));
}

void HilinkTcpOutduct::SetOnFailedBundleVecSendCallback(const OnFailedBundleVecSendCallback_t& callback) {
    m_stcpBundleSource.SetOnFailedBundleVecSendCallback(callback);
}
void HilinkTcpOutduct::SetOnFailedBundleZmqSendCallback(const OnFailedBundleZmqSendCallback_t& callback) {
    m_stcpBundleSource.SetOnFailedBundleZmqSendCallback(callback);
}
void HilinkTcpOutduct::SetOnSuccessfulBundleSendCallback(const OnSuccessfulBundleSendCallback_t& callback) {
    m_stcpBundleSource.SetOnSuccessfulBundleSendCallback(callback);
}
void HilinkTcpOutduct::SetOnOutductLinkStatusChangedCallback(const OnOutductLinkStatusChangedCallback_t& callback) {
    m_stcpBundleSource.SetOnOutductLinkStatusChangedCallback(callback);
}
void HilinkTcpOutduct::SetUserAssignedUuid(uint64_t userAssignedUuid) {
    m_stcpBundleSource.SetUserAssignedUuid(userAssignedUuid);
}

void HilinkTcpOutduct::Connect() {
    m_stcpBundleSource.Connect(m_outductConfig.remoteHostname, boost::lexical_cast<std::string>(m_outductConfig.remotePort));
}
bool HilinkTcpOutduct::ReadyToForward() {
    return m_stcpBundleSource.ReadyToForward();
}
void HilinkTcpOutduct::Stop() {
    m_stcpBundleSource.Stop();
}
void HilinkTcpOutduct::GetOutductFinalStats(OutductFinalStats & finalStats) {
    finalStats.m_convergenceLayer = m_outductConfig.convergenceLayer;
    finalStats.m_totalBundlesAcked = m_stcpBundleSource.GetTotalBundlesAcked();
    finalStats.m_totalBundlesSent = m_stcpBundleSource.GetTotalBundlesSent();
}
void HilinkTcpOutduct::PopulateOutductTelemetry(std::unique_ptr<OutductTelemetry_t>& outductTelem) {
    std::unique_ptr<StcpOutductTelemetry_t> t = boost::make_unique<StcpOutductTelemetry_t>();
    m_stcpBundleSource.GetTelemetry(*t);
    outductTelem = std::move(t);
    outductTelem->m_linkIsUpPerTimeSchedule = m_linkIsUpPerTimeSchedule;
}
