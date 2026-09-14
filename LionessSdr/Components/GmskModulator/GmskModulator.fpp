module LionessSdr {
    @ Applies GMSK modulation to the incoming frames for transmission
    active component GmskModulator {

        @ Input port: Framer Bytes (Framer sends ComDataWithContext)
        async input port framerIn: Svc.ComDataWithContext

        @ Output port for returning ownership of Framer buffers
        output port framerReturnOut: Svc.ComDataWithContext

        @ Output port for indicating status to the Framer
        output port framerStatusOut: Fw.SuccessCondition

        @ Output port: I/Q Samples to the IIO Driver
        output port driverOut: Drv.ByteStreamSend

        @ Input port to receive the ready signal from the driver
        async input port drvConnected: Drv.ByteStreamReady

        @ Buffer manager port for allocating I/Q sample buffers
        output port bufferAllocate: Fw.BufferGet

        @ Buffer manager port for freeing buffers from the driver (if needed)
        output port bufferDeallocate: Fw.BufferSend

        @ Sample Rate in Hz
        param SAMPLE_RATE: U32 default 2000000

        @ Symbol Rate in Hz
        param SYMBOL_RATE: U32 default 20000

        @ BT Product (Gaussian filter bandwidth times symbol rate)
        param BT: F32 default 0.3

        @ Output power in dB
        param OUTPUT_POWER_DB: F32 default -10.0

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