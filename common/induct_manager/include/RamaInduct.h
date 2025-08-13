#ifndef RAMA_INDUCT_H
#define RAMA_INDUCT_H 1

#include <string>
#include "Induct.h"
#include "UdpBundleSink.h"
#include <memory>

class CLASS_VISIBILITY_INDUCT_MANAGER_LIB RamaInduct : public Induct {
public:
    INDUCT_MANAGER_LIB_EXPORT RamaInduct(const InductProcessBundleCallback_t & inductProcessBundleCallback, const induct_element_config_t & inductConfig);
    INDUCT_MANAGER_LIB_EXPORT virtual ~RamaInduct() override;
    INDUCT_MANAGER_LIB_EXPORT virtual void PopulateInductTelemetry(InductTelemetry_t& inductTelem) override;
private:
    RamaInduct();
    INDUCT_MANAGER_LIB_EXPORT void HandleRamaBundle(padded_vector_uint8_t & bundle);
    INDUCT_MANAGER_LIB_EXPORT void ConnectionReadyToBeDeletedNotificationReceived();
    INDUCT_MANAGER_LIB_EXPORT void RemoveInactiveConnection();

    boost::asio::io_service m_ioService;
    std::unique_ptr<boost::thread> m_ioServiceThreadPtr;
    std::unique_ptr<UdpBundleSink> m_udpBundleSinkPtr;
};

#endif // RAMA_INDUCT_H
