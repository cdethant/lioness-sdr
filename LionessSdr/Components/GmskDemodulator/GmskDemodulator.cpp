// ======================================================================
// \title  GmskDemodulator.cpp
// \author ethant
// \brief  cpp file for GmskDemodulator component implementation class
// ======================================================================

#include "LionessSdr/Components/GmskDemodulator/GmskDemodulator.hpp"
#include <liquid/liquid.h>

namespace LionessSdr {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

GmskDemodulator ::GmskDemodulator(const char* const compName) : GmskDemodulatorComponentBase(compName) {}

GmskDemodulator ::~GmskDemodulator() {}

// ----------------------------------------------------------------------
// Parameter update handlers
// ----------------------------------------------------------------------

void GmskDemodulator ::parametersLoaded() {
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
        if (m_demod != nullptr) {
            gmskdem_destroy(static_cast<gmskdem>(m_demod));
        }
        m_demod = gmskdem_create(k, m, bt);
    }
}

void GmskDemodulator ::parameterUpdated(FwPrmIdType id) {
    // If any parameter changes, reload the DSP filter
    parametersLoaded();
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void GmskDemodulator ::driverIn_handler(FwIndexType portNum, Fw::Buffer& buffer, const Drv::ByteStreamStatus& status) {
    if (m_demod == nullptr || !buffer.isValid() || buffer.getSize() == 0) {
        // Demodulator not initialized or invalid buffer, drop packet and return buffer
        if (isConnected_driverReturnOut_OutputPort(0)) {
            driverReturnOut_out(0, buffer);
        }
        return;
    }

    Fw::ParamValid valid;
    U32 symbolRate = paramGet_SYMBOL_RATE(valid);
    U32 sampleRate = paramGet_SAMPLE_RATE(valid);
    unsigned int k = sampleRate / symbolRate;

    if (k == 0) {
        if (isConnected_driverReturnOut_OutputPort(0)) {
            driverReturnOut_out(0, buffer);
        }
        return;
    }

    // Input buffer size calculation
    U32 numComplexSamples = buffer.getSize() / sizeof(float _Complex);
    U32 numBits = numComplexSamples / k;
    U32 numBytes = numBits / 8;

    if (numBytes == 0) {
        if (isConnected_driverReturnOut_OutputPort(0)) {
            driverReturnOut_out(0, buffer);
        }
        return;
    }

    // Request an output buffer for the demodulated bytes
    Fw::Buffer outBuffer = bufferAllocate_out(0, numBytes);
    if (!outBuffer.isValid()) {
        if (isConnected_driverReturnOut_OutputPort(0)) {
            driverReturnOut_out(0, buffer);
        }
        return;
    }

    float _Complex* inPtr = reinterpret_cast<float _Complex*>(buffer.getData());
    U8* outPtr = outBuffer.getData();
    U32 inIdx = 0;

    // Demodulate bit-by-bit and pack into bytes
    for (U32 i = 0; i < numBytes; i++) {
        U8 byte = 0;
        for (U32 bit = 0; bit < 8; bit++) {
            unsigned int sym = 0;
            gmskdem_demodulate(static_cast<gmskdem>(m_demod), reinterpret_cast<liquid_float_complex*>(&inPtr[inIdx]),
                               &sym);

            // Extract bit (sym is 0 or 1), pack MSB first
            byte = (byte << 1) | (sym & 1);
            inIdx += k;
        }
        outPtr[i] = byte;
    }

    outBuffer.setSize(numBytes);

    // Send the frame data to the comm driver
    if (isConnected_comSendOut_OutputPort(0)) {
        (void) comSendOut_out(0, outBuffer);
    } 

    // ByteStreamSend is synchronous and caller retains ownership, so we always deallocate
    if (isConnected_bufferDeallocate_OutputPort(0)) {
        bufferDeallocate_out(0, outBuffer);
    }

    // Return the original SDR buffer back to the driver
    if (isConnected_driverReturnOut_OutputPort(0)) {
        driverReturnOut_out(0, buffer);
    }
}

void GmskDemodulator ::drvConnected_handler(FwIndexType portNum) {
    // Optionally use this to pause transmission until the hardware is ready.
    // For now, do nothing and just acknowledge it.
}

}  // namespace LionessSdr
