// ======================================================================
// \title  LinuxIioDriver.cpp
// \author ethant
// \brief  cpp file for LinuxIioDriver component implementation class
// ======================================================================

#include "LionessSdr/Components/LinuxIioDriver/LinuxIioDriver.hpp"

namespace LionessSdr {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

LinuxIioDriver ::LinuxIioDriver(const char* const compName) : LinuxIioDriverComponentBase(compName) {}

LinuxIioDriver ::~LinuxIioDriver() {
    if (this->m_tx_buffer != nullptr) {
        iio_buffer_destroy(this->m_tx_buffer);
    }
    if (this->m_ctx != nullptr) {
        iio_context_destroy(this->m_ctx);
    }
}

// ----------------------------------------------------------------------
// Hardware Initialization
// ----------------------------------------------------------------------

bool LinuxIioDriver ::setup(const char* uri) {
    Fw::String fwUri(uri);

    this->m_ctx = iio_create_context_from_uri(uri);
    if (this->m_ctx == nullptr) {
        this->log_WARNING_HI_IioConfigError(fwUri, -1);
        return false;
    }

    // Find the Pluto SDR TX device
    this->m_tx_dev = iio_context_find_device(this->m_ctx, "cf-ad9361-dds-core-lpc");
    if (this->m_tx_dev == nullptr) {
        this->log_WARNING_HI_IioConfigError(fwUri, -2);
        return false;
    }

    // We MUST enable at least one channel before creating a buffer, otherwise it fails!
    this->m_tx_channel = iio_device_find_channel(this->m_tx_dev, "voltage0", true);
    if (this->m_tx_channel != nullptr) {
        iio_channel_enable(this->m_tx_channel);
    }
    struct iio_channel* q_channel = iio_device_find_channel(this->m_tx_dev, "voltage1", true);
    if (q_channel != nullptr) {
        iio_channel_enable(q_channel);
    }

    // Create the IIO buffer
    this->m_tx_buffer = iio_device_create_buffer(this->m_tx_dev, 4096, false);
    if (this->m_tx_buffer == nullptr) {
        this->log_WARNING_HI_IioConfigError(fwUri, -3);
        return false;
    }

    this->log_ACTIVITY_HI_Configured(fwUri);
    if (this->isConnected_ready_OutputPort(0)) {
        this->ready_out(0);
    }
    return true;
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

Drv::ByteStreamStatus LinuxIioDriver ::dataIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    if (this->m_tx_enabled && this->m_tx_buffer != nullptr) {
        // Push the buffer memory to the IIO buffer using libiio APIs.
        memcpy(iio_buffer_first(this->m_tx_buffer, this->m_tx_channel), fwBuffer.getData(), fwBuffer.getSize());
        iio_buffer_push(this->m_tx_buffer);

        // Update telemetry
        this->tlmWrite_BuffersReceived(1);
        this->tlmWrite_BytesTransmitted(fwBuffer.getSize());
    }

    return Drv::ByteStreamStatus::OP_OK;
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void LinuxIioDriver ::ENABLE_TX_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On enable) {
    this->m_tx_enabled = (enable == Fw::On::ON);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// ----------------------------------------------------------------------
// Parameter Handlers
// ----------------------------------------------------------------------

void LinuxIioDriver ::parametersLoaded() {
    // This is called automatically on boot if a parameter file is loaded
    if (this->m_ctx != nullptr) {
        Fw::ParamValid valid;
        U64 lo_freq = this->paramGet_TX_LO_FREQ(valid);
        if (valid == Fw::ParamValid::VALID) {
            // Apply the loaded LO frequency to the SDR hardware
            struct iio_device* phy = iio_context_find_device(this->m_ctx, "ad9361-phy");
            iio_channel_attr_write_longlong(iio_device_find_channel(phy, "altvoltage1", true), "frequency", lo_freq);
        }
    }
}

void LinuxIioDriver ::parameterUpdated(FwPrmIdType id) {
    // This is called when the GDS updates a parameter via command
    if (this->m_ctx == nullptr) {
        return;  // Hardware not initialized yet
    }

    switch (id) {
        case PARAMID_TX_LO_FREQ:
            // TODO: Apply new LO frequency
            break;
        case PARAMID_TX_SAMP_RATE:
            // TODO: Apply new sample rate
            break;
        case PARAMID_TX_ATTENUATION:
            // TODO: Apply new attenuation
            break;
        default:
            break;
    }
}

}  // namespace LionessSdr
