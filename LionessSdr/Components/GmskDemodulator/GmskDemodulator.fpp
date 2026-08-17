module LionessSdr {
    @ Applies GMSK demodulation to received IQ data
    active component GmskDemodulator {

        @ Input port: I/Q Samples from RTL
        async input port driverIn: Drv.ByteStreamData

        @ Output port for returning ownership of I/Q sample buffers to the driver
        output port driverReturnOut: Fw.BufferSend

        @ Output port: demodulated data to the communications driver
        output port comSendOut: Drv.ByteStreamSend

        @ Input port to receive the ready signal from the driver
        async input port drvConnected: Drv.ByteStreamReady

        @ Buffer manager port for allocating demodulated data buffers
        output port bufferAllocate: Fw.BufferGet

        @ Buffer manager port for freeing demodulated data buffers
        output port bufferDeallocate: Fw.BufferSend

        @ Sample Rate in Hz
        param SAMPLE_RATE: U32 default 2000000

        @ Symbol Rate in Hz
        param SYMBOL_RATE: U32 default 100000

        @ BT Product (Gaussian filter bandwidth times symbol rate)
        param BT: F32 default 0.3

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