// ======================================================================
// \title  GmskModulator.cpp
// \author ethant
// \brief  cpp file for GmskModulator component implementation class
// ======================================================================

#include "LionessSdr/Components/GmskModulator/GmskModulator.hpp"
#include <liquid/liquid.h>

namespace LionessSdr {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

GmskModulator ::GmskModulator(const char* const compName) : GmskModulatorComponentBase(compName), m_mod(nullptr) {}

GmskModulator ::~GmskModulator() {
    if (m_mod != nullptr) {
        gmskmod_destroy(static_cast<gmskmod>(m_mod));
        m_mod = nullptr;
    }
}

// ----------------------------------------------------------------------
// Parameter update handlers
// ----------------------------------------------------------------------

void GmskModulator ::parametersLoaded() {
    // Recreate the DSP object whenever parameters are loaded
    Fw::ParamValid valid;
    U32 symbolRate = paramGet_SYMBOL_RATE(valid);
    U32 sampleRate = paramGet_SAMPLE_RATE(valid);
    F32 bt = paramGet_BT(valid);

    // Filter properties
    unsigned int k = sampleRate / symbolRate;
    unsigned int m = 3;  // filter delay (symbols)

    // Ensure valid parameters before creating
    if (k > 0 && bt > 0.0f) {
        if (m_mod != nullptr) {
            gmskmod_destroy(static_cast<gmskmod>(m_mod));
        }
        m_mod = gmskmod_create(k, m, bt);
    }
}

void GmskModulator ::parameterUpdated(FwPrmIdType id) {
    // If any parameter changes, reload the DSP filter
    parametersLoaded();
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void GmskModulator ::framerIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    if (m_mod == nullptr) {
        // Modulator not initialized, drop packet and return status
        if (isConnected_framerReturnOut_OutputPort(0)) {
            framerReturnOut_out(0, data, context);
        }
        if (isConnected_framerStatusOut_OutputPort(0)) {
            Fw::Success status(Fw::Success::FAILURE);
            framerStatusOut_out(0, status);
        }
        return;
    }

    Fw::ParamValid valid;
    U32 symbolRate = paramGet_SYMBOL_RATE(valid);
    U32 sampleRate = paramGet_SAMPLE_RATE(valid);
    unsigned int k = sampleRate / symbolRate;

    // Output buffer size calculation (8 bits per byte * k samples per bit)
    U32 outSamplesPerByte = 8 * k;
    U32 numBytes = data.getSize();
    U32 outBufferSize = numBytes * outSamplesPerByte * sizeof(float _Complex);  // liquid-dsp uses float _Complex

    // Request an output buffer
    Fw::Buffer outBuffer = bufferAllocate_out(0, outBufferSize);
    if (!outBuffer.isValid()) {
        if (isConnected_framerReturnOut_OutputPort(0)) {
            framerReturnOut_out(0, data, context);
        }
        if (isConnected_framerStatusOut_OutputPort(0)) {
            Fw::Success status(Fw::Success::FAILURE);
            framerStatusOut_out(0, status);
        }
        return;
    }

    U8* inPtr = data.getData();
    float _Complex* outPtr = reinterpret_cast<float _Complex*>(outBuffer.getData());
    U32 outIdx = 0;

    // Modulate bit-by-bit
    for (U32 i = 0; i < numBytes; i++) {
        U8 byte = inPtr[i];
        for (U32 bit = 0; bit < 8; bit++) {
            // Extract MSB first
            unsigned int sym = (byte >> (7 - bit)) & 1;
            gmskmod_modulate(static_cast<gmskmod>(m_mod), sym,
                             reinterpret_cast<liquid_float_complex*>(&outPtr[outIdx]));
            outIdx += k;
        }
    }

    outBuffer.setSize(outIdx * sizeof(float _Complex));

    // Send the IQ samples to the driver
    if (isConnected_driverOut_OutputPort(0)) {
        driverOut_out(0, outBuffer);
    } else {
        // Free the buffer if it can't be sent
        bufferDeallocate_out(0, outBuffer);
    }

    // Return the original buffer back to the framer
    if (isConnected_framerReturnOut_OutputPort(0)) {
        framerReturnOut_out(0, data, context);
    }

    if (this->isConnected_framerStatusOut_OutputPort(0)) {
        Fw::Success status(Fw::Success::SUCCESS);
        framerStatusOut_out(0, status);
    }
}

void GmskModulator ::drvConnected_handler(FwIndexType portNum) {
    // Optionally use this to pause transmission until the hardware is ready.
    // For now, do nothing and just acknowledge it.
}

}  // namespace LionessSdr
