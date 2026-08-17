module LionessSdr {
    @ Driver for Pluto SDR transmission
    active component LinuxIioDriver {
        # This command might be evolved to a setup of the LO freq and other params of the SDR.
        @ Command to enable or disable transmission
        async command ENABLE_TX(
            enable: Fw.On
        )

        @ Port to receive raw streaming data from upstream components
        guarded input port dataIn: Drv.ByteStreamSend


        @ Port to signal that the driver is ready
        output port ready: Drv.ByteStreamReady

        @ Telemetry for status tracking
        telemetry BuffersReceived: U32
        telemetry BytesTransmitted: U64

        @ URI for the SDR (e.g. ip:192.168.3.1)
        param URI: string size 256 default "ip:192.168.3.1"

        @ TX Local Oscillator Frequency in Hz
        param TX_LO_FREQ: U64 default 2400000000

        @ Baseband Sampling Rate in Hz
        param TX_SAMP_RATE: U32 default 2000000

        @ TX Attenuation in dB
        param TX_ATTENUATION: U32 default 10

        @ Event for configuration status
        event Configured(uri: string size 256) \
            severity activity high \
            id 0 \
            format "IIO Driver configured with URI: {}"

        @ Error handling events
        event IioConfigError(uri: string size 256, error_code: I32) \
            severity warning high \
            id 1 \
            format "Failed to configure IIO driver on URI {} with error {}"

        event TransmitError(error_code: I32) \
            severity warning high \
            id 2 \
            format "IIO transmission failed with error {}"

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
