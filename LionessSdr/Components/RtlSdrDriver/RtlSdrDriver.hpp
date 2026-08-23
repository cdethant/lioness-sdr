// ======================================================================
// \title  RtlSdrDriver.hpp
// \author ethant
// \brief  hpp file for RtlSdrDriver component implementation class
// ======================================================================

#ifndef LionessSdr_RtlSdrDriver_HPP
#define LionessSdr_RtlSdrDriver_HPP

#include "LionessSdr/Components/RtlSdrDriver/RtlSdrDriverComponentAc.hpp"
#include <rtl-sdr.h>
#include <Os/Task.hpp>

namespace LionessSdr {

class RtlSdrDriver final : public RtlSdrDriverComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct RtlSdrDriver object
    RtlSdrDriver(const char* const compName  //!< The component name
    );

    //! Destroy RtlSdrDriver object
    ~RtlSdrDriver();

    //! Initialize the RTL-SDR hardware and start the receive task
    //! \return true if successful, false otherwise
    bool setup();

    //! Parameter loaded handler
    void parametersLoaded() override;

    //! Parameter updated handler
    void parameterUpdated(FwPrmIdType id) override;

  private:
    rtlsdr_dev_t* m_dev = nullptr;
    Os::Task m_rxTask;

    static void rxTaskEntry(void* ptr);
    static void rxCallback(unsigned char* buf, uint32_t len, void* ctx);

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for dataReturnIn
    //!
    //! Port to receive buffer allocations back from downstream
    void dataReturnIn_handler(FwIndexType portNum,  //!< The port number
                              Fw::Buffer& fwBuffer  //!< The buffer
                              ) override;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command ENABLE_RX
    //!
    //! Command to enable or disable reception
    void ENABLE_RX_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                              U32 cmdSeq,           //!< The command sequence number
                              Fw::On enable) override;
};

}  // namespace LionessSdr

#endif
