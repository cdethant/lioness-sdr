module LionessSdr {
    @ RX side driver for thr RTL SDR
    active component RtlSdrDriver {

        @ Command to enable or disable reception
        async command ENABLE_RX(
            enable: Fw.On
        )

        @ Port to send raw streaming data to downstream components
        output port dataOut: Drv.ByteStreamData

        @ Port to signal that the driver is ready
        output port ready: Drv.ByteStreamReady

        @ Port to receive buffer allocations back from downstream
        guarded input port dataReturnIn: Fw.BufferSend

        @ Buffer manager port for allocating I/Q sample buffers
        output port bufferAllocate: Fw.BufferGet

        @ Buffer manager port for freeing buffers (if needed)
        output port bufferDeallocate: Fw.BufferSend

        @ Telemetry for status tracking
        telemetry BuffersSent: U32
        telemetry BytesReceived: U64

        @ URI for the SDR (e.g. ip:192.168.3.1)
        param URI: string size 256 default "ip:192.168.3.1"

        @ RX Local Oscillator Frequency in Hz
        param RX_LO_FREQ: U64 default 2400000000

        @ Baseband Sampling Rate in Hz
        param RX_SAMP_RATE: U32 default 2000000

        @ RX Gain in dB
        param RX_GAIN: U32 default 20

        @ Event for configuration status
        event Configured(uri: string size 256) \
            severity activity high \
            id 0 \
            format "RTL-SDR Driver configured with URI: {}"

        @ Error handling events
        event RtlSdrConfigError(uri: string size 256, error_code: I32) \
            severity warning high \
            id 1 \
            format "Failed to configure RTL-SDR driver on URI {} with error {}"

        event ReceiveError(error_code: I32) \
            severity warning high \
            id 2 \
            format "RTL-SDR reception failed with error {}"

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Enables command handling
        import Fw.Command

        @ Enables event handling
        import Fw.Event

        @ Enables telemetry channels handling
        import Fw.Channel

        @ Port to return the value of a parameter
        param get port prmGetOut

        @Port to set the value of a parameter
        param set port prmSetOut

    }
}