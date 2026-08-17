// ======================================================================
// \title  LinuxIioDriver.hpp
// \author ethant
// \brief  hpp file for LinuxIioDriver component implementation class
// ======================================================================

#ifndef LionessSdr_LinuxIioDriver_HPP
#define LionessSdr_LinuxIioDriver_HPP

#include "LionessSdr/Components/LinuxIioDriver/LinuxIioDriverComponentAc.hpp"
#include <iio.h>

namespace LionessSdr {

class LinuxIioDriver final : public LinuxIioDriverComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct LinuxIioDriver object
    LinuxIioDriver(const char* const compName  //!< The component name
    );

    //! Destroy LinuxIioDriver object
    ~LinuxIioDriver();

    // ----------------------------------------------------------------------
    // Hardware Initialization
    // ----------------------------------------------------------------------

    //! Initialize the IIO hardware context
    //! \return true if successful, false otherwise
    bool setup(const char* uri);

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for dataIn
    //!
    //! Port to receive raw streaming data from upstream components
    Drv::ByteStreamStatus dataIn_handler(FwIndexType portNum,  //!< The port number
                        Fw::Buffer& fwBuffer  //!< The buffer
                        ) override;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command ENABLE_TX
    //!
    //! Command to enable or disable transmission
    void ENABLE_TX_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                              U32 cmdSeq,           //!< The command sequence number
                              Fw::On enable) override;

    // ----------------------------------------------------------------------
    // Parameter Handlers
    // ----------------------------------------------------------------------

    //! Called when parameters are successfully loaded on boot
    void parametersLoaded() override;

    //! Called when a parameter is updated from the ground via command
    void parameterUpdated(FwPrmIdType id) override;

    // ----------------------------------------------------------------------
    // Hardware State and Context
    // ----------------------------------------------------------------------

    struct iio_context* m_ctx = nullptr;
    struct iio_device* m_tx_dev = nullptr;
    struct iio_channel* m_tx_channel = nullptr;
    struct iio_buffer* m_tx_buffer = nullptr;

    bool m_tx_enabled = true;  //!< Tracks if transmission is enabled via command (hardcoded for TX testing)
};

}  // namespace LionessSdr

#endif
