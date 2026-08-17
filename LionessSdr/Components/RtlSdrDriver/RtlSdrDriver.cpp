// ======================================================================
// \title  RtlSdrDriver.cpp
// \author ethant
// \brief  cpp file for RtlSdrDriver component implementation class
// ======================================================================

#include "LionessSdr/Components/RtlSdrDriver/RtlSdrDriver.hpp"

namespace LionessSdr {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

RtlSdrDriver ::RtlSdrDriver(const char* const compName) : RtlSdrDriverComponentBase(compName) {}

RtlSdrDriver ::~RtlSdrDriver() {
    if (m_dev != nullptr) {
        rtlsdr_cancel_async(m_dev);
        m_rxTask.join();
        rtlsdr_close(m_dev);
        m_dev = nullptr;
    }
}

// ----------------------------------------------------------------------
// Parameter handlers
// ----------------------------------------------------------------------

void RtlSdrDriver ::parametersLoaded() {
    if (m_dev == nullptr) {
        return; // Only apply if device is already open
    }

    Fw::ParamValid valid;
    U64 rxLoFreq = paramGet_RX_LO_FREQ(valid);
    U32 rxSampRate = paramGet_RX_SAMP_RATE(valid);
    U32 rxGain = paramGet_RX_GAIN(valid);

    rtlsdr_set_center_freq(m_dev, static_cast<uint32_t>(rxLoFreq));
    rtlsdr_set_sample_rate(m_dev, rxSampRate);
    rtlsdr_set_tuner_gain_mode(m_dev, 1); // Manual gain
    rtlsdr_set_tuner_gain(m_dev, rxGain * 10); // librtlsdr uses tenths of a dB
}

void RtlSdrDriver ::parameterUpdated(FwPrmIdType id) {
    parametersLoaded();
}

// ----------------------------------------------------------------------
// Background Task
// ----------------------------------------------------------------------

void RtlSdrDriver ::rxCallback(unsigned char* buf, uint32_t len, void* ctx) {
    RtlSdrDriver* comp = static_cast<RtlSdrDriver*>(ctx);

    if (comp->isConnected_bufferAllocate_OutputPort(0)) {
        Fw::Buffer fwBuf = comp->bufferAllocate_out(0, len);
        if (fwBuf.isValid()) {
            memcpy(fwBuf.getData(), buf, len);
            fwBuf.setSize(len);
            
            if (comp->isConnected_dataOut_OutputPort(0)) {
                // Send downstream
                comp->dataOut_out(0, fwBuf, Drv::ByteStreamStatus::OP_OK);
                comp->tlmWrite_BytesReceived(len);
            } else {
                // If not connected to dataOut, immediately deallocate it
                comp->bufferDeallocate_out(0, fwBuf);
            }
        }
    }
}

void RtlSdrDriver ::rxTaskEntry(void* ptr) {
    RtlSdrDriver* comp = static_cast<RtlSdrDriver*>(ptr);
    if (comp->m_dev != nullptr) {
        // Blocks until rtlsdr_cancel_async is called
        rtlsdr_read_async(comp->m_dev, rxCallback, comp, 0, 0);
    }
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void RtlSdrDriver ::dataReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    if (isConnected_bufferDeallocate_OutputPort(0)) {
        bufferDeallocate_out(0, fwBuffer);
    }
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void RtlSdrDriver ::ENABLE_RX_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On enable) {
    if (enable == Fw::On::ON) {
        if (m_dev == nullptr) {
            int ret = rtlsdr_open(&m_dev, 0);
            if (ret < 0) {
                log_WARNING_HI_RtlSdrConfigError(Fw::String("0"), ret);
                this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
                return;
            }
            
            // Configure initial parameters
            parametersLoaded();
            rtlsdr_reset_buffer(m_dev);

            // Start the background thread
            Os::Task::Status status = m_rxTask.start(Fw::String("RtlSdrRx"), rxTaskEntry, this, 40);
            if (status != Os::Task::OP_OK) {
                rtlsdr_close(m_dev);
                m_dev = nullptr;
                this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
                return;
            }

            log_ACTIVITY_HI_Configured(Fw::String("0"));
            
            if (isConnected_ready_OutputPort(0)) {
                ready_out(0);
            }
        }
    } else {
        if (m_dev != nullptr) {
            rtlsdr_cancel_async(m_dev);
            m_rxTask.join();
            rtlsdr_close(m_dev);
            m_dev = nullptr;
        }
    }

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace LionessSdr
