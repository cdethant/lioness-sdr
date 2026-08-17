// ======================================================================
// \title  GmskModulator.hpp
// \author ethant
// \brief  hpp file for GmskModulator component implementation class
// ======================================================================

#ifndef LionessSdr_GmskModulator_HPP
#define LionessSdr_GmskModulator_HPP

#include "LionessSdr/Components/GmskModulator/GmskModulatorComponentAc.hpp"

namespace LionessSdr {

class GmskModulator final : public GmskModulatorComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct GmskModulator object
    GmskModulator(const char* const compName  //!< The component name
    );

    //! Destroy GmskModulator object
    ~GmskModulator();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for framerIn
    //!
    //! Input port: Framer Bytes (Framer sends ComDataWithContext)
    void framerIn_handler(FwIndexType portNum,  //!< The port number
                          Fw::Buffer& data,
                          const ComCfg::FrameContext& context) override;

    //! Handler implementation for drvConnected
    //!
    //! Input port to receive the ready signal from the driver
    void drvConnected_handler(FwIndexType portNum) override;

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
    
    //! The liquid-dsp GMSK modulator object
    void* m_mod = nullptr;
};

}  // namespace LionessSdr

#endif
