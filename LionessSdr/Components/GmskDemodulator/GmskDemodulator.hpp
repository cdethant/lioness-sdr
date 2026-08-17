// ======================================================================
// \title  GmskDemodulator.hpp
// \author ethant
// \brief  hpp file for GmskDemodulator component implementation class
// ======================================================================

#ifndef LionessSdr_GmskDemodulator_HPP
#define LionessSdr_GmskDemodulator_HPP

#include "LionessSdr/Components/GmskDemodulator/GmskDemodulatorComponentAc.hpp"

namespace LionessSdr {

class GmskDemodulator final : public GmskDemodulatorComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct GmskDemodulator object
    GmskDemodulator(const char* const compName  //!< The component name
    );

    //! Destroy GmskDemodulator object
    ~GmskDemodulator();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for driverIn
    //!
    //! Input port: I/Q Samples from RTL
    void driverIn_handler(FwIndexType portNum,  //!< The port number
                          Fw::Buffer& buffer,
                          const Drv::ByteStreamStatus& status) override;

    //! Handler implementation for drvConnected
    //!
    //! Input port to receive the ready signal from the driver
    void drvConnected_handler(FwIndexType portNum  //!< The port number
                              ) override;

  protected:
    // ----------------------------------------------------------------------
    // Parameter update handlers
    // ----------------------------------------------------------------------

    //! Called when parameters are successfully loaded
    void parametersLoaded() override;

    //! Called whenever a parameter is updated from the ground
    //! We need this to recreate the DSP object if the BT or sample rate changes
    void parameterUpdated(FwPrmIdType id) override;

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------
    
    //! The liquid-dsp GMSK demodulator object
    void* m_demod = nullptr;
};

}  // namespace LionessSdr

#endif
