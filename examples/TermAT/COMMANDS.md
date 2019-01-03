

# Examples of AT command for SIM900

For full list see [SIM900_AT Command Manual](https://simcom.ee/documents/SIM900/SIM900_AT%20Command%20Manual_V1.11.pdf)


| Execution Command  | Description | Expected Response |
| --- | --- | --- |
| **AT** | The AT Command interpreter actively responds to input. | **OK** |
| **ATI** | Display the product name and the product release information. | **SIM900 R11.0** |
| **AT+GSV** | Display product identification information: the manufacturer, the product name and the product revision information. | **OK** |
| **ATE<value>** | This setting determines whether or not the TA echoes characters received from TE during Command state. Parameter **<value>**: _0_ echo mode off, _1_ echo mode on. | **OK** |
| **AT+IPR=?** | Returns list of supported auto detectable **<rates1>** and list of supported fixed-only **<rates2>** | **+IPR: (<rates1>),(<rates2>)** **OK** |
| **AT+IPR?** | Returns current baud rate. | **+IPR: <rate>** **OK** |
| **AT+IPR=<rate>** | This parameter setting determines the data rate of the TA on the serial interface. The rate of Command takes effect following the issuance of any result code associated with the current Command line. Parameter **<rate>* Baud rate per second: _0_ (Auto-bauding) _1200_ _2400_ _4800_ _9600_ _19200_ _38400_ _57600_ _115200_ | **OK** |
