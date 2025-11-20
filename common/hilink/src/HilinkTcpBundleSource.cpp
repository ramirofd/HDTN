#include "HilinkTcpBundleSource.h"
#include <cstring>

HilinkTcpBundleSource::HilinkTcpBundleSource(uint8_t headerByte, uint8_t trailerByte,
    uint16_t keepAliveIntervalSeconds, unsigned int maxUnacked)
    : m_headerByte(headerByte),
    m_trailerByte(trailerByte),
    m_stcpBundleSource(keepAliveIntervalSeconds, maxUnacked) {}

HilinkTcpBundleSource::~HilinkTcpBundleSource() {}

bool HilinkTcpBundleSource::Forward(const uint8_t* bundleData, std::size_t size, std::vector<uint8_t>&& userData) {
    const std::size_t encodedSize = size + 2;
    padded_vector_uint8_t data(encodedSize);
    data[0] = m_headerByte;
    if (size > 0) {
        std::memcpy(data.data() + 1, bundleData, size);
    }
    data[encodedSize - 1] = m_trailerByte;
    return ForwardVector(data, std::move(userData));
}

bool HilinkTcpBundleSource::Forward(zmq::message_t & movableDataZmq, std::vector<uint8_t>&& userData) {
    const std::size_t encodedSize = movableDataZmq.size() + 2;
    zmq::message_t data(encodedSize);
    uint8_t* ptr = static_cast<uint8_t*>(data.data());
    ptr[0] = m_headerByte;
    if (movableDataZmq.size() > 0) {
        std::memcpy(ptr + 1, movableDataZmq.data(), movableDataZmq.size());
    }
    ptr[encodedSize - 1] = m_trailerByte;
    return m_stcpBundleSource.Forward(data, std::move(userData));
}

bool HilinkTcpBundleSource::Forward(padded_vector_uint8_t& movableDataVec, std::vector<uint8_t>&& userData) {
    movableDataVec.insert(movableDataVec.begin(), m_headerByte);
    movableDataVec.push_back(m_trailerByte);
    return ForwardVector(movableDataVec, std::move(userData));
}

bool HilinkTcpBundleSource::ForwardVector(padded_vector_uint8_t& data, std::vector<uint8_t>&& userData) {
    return m_stcpBundleSource.Forward(data, std::move(userData));
}

void HilinkTcpBundleSource::SetOnFailedBundleVecSendCallback(const OnFailedBundleVecSendCallback_t& callback) {
    m_stcpBundleSource.SetOnFailedBundleVecSendCallback(callback);
}

void HilinkTcpBundleSource::SetOnFailedBundleZmqSendCallback(const OnFailedBundleZmqSendCallback_t& callback) {
    m_stcpBundleSource.SetOnFailedBundleZmqSendCallback(callback);
}

void HilinkTcpBundleSource::SetOnSuccessfulBundleSendCallback(const OnSuccessfulBundleSendCallback_t& callback) {
    m_stcpBundleSource.SetOnSuccessfulBundleSendCallback(callback);
}

void HilinkTcpBundleSource::SetOnOutductLinkStatusChangedCallback(const OnOutductLinkStatusChangedCallback_t& callback) {
    m_stcpBundleSource.SetOnOutductLinkStatusChangedCallback(callback);
}

void HilinkTcpBundleSource::SetUserAssignedUuid(uint64_t userAssignedUuid) {
    m_stcpBundleSource.SetUserAssignedUuid(userAssignedUuid);
}

void HilinkTcpBundleSource::Connect(const std::string & hostname, const std::string & port) {
    m_stcpBundleSource.Connect(hostname, port);
}

bool HilinkTcpBundleSource::ReadyToForward() {
    return m_stcpBundleSource.ReadyToForward();
}

void HilinkTcpBundleSource::Stop() {
    m_stcpBundleSource.Stop();
}

std::size_t HilinkTcpBundleSource::GetTotalBundlesAcked() const noexcept {
    return m_stcpBundleSource.GetTotalBundlesAcked();
}

std::size_t HilinkTcpBundleSource::GetTotalBundlesSent() const noexcept {
    return m_stcpBundleSource.GetTotalBundlesSent();
}

std::size_t HilinkTcpBundleSource::GetTotalBundlesUnacked() const noexcept {
    return m_stcpBundleSource.GetTotalBundlesUnacked();
}

void HilinkTcpBundleSource::GetTelemetry(StcpOutductTelemetry_t& telem) const {
    m_stcpBundleSource.GetTelemetry(telem);
}
