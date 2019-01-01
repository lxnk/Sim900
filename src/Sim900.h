/*
Sim900.h -
DESCRIPTION
*/

#ifndef Sim900_h
#define Sim900_h

//#include <inttypes.h>
#include <SoftwareSerial.h>
#include <SD.h>


/******************************************************************************
* Definitions
******************************************************************************/

//#define _SS_MAX_RX_BUFF 64 // RX buffer size
//#ifndef GCC_VERSION
//#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
//#endif

//#define RESPOSE_SEPARATOR 1



class Sim900: public SoftwareSerial
{
protected:
    
  /*  const char* patterns[7] = { "\r\n",
                                "> ",
                                "\r\nOK\r\n",
                                "\r\nERROR\r\n",
                                "\r\n+SMTPFT: 1,",
                                "\r\n+SMTPFT: 1,",
                                "\r\n+SMTPSEND: 1\r\n"};
    */
   // const char PROGMEM b64_alphabet[] = "AB""ABCDEFGHIJKLMNOPQRSTUVWXYZ"
   //                                     "abcdefghijklmnopqrstuvwxyz"
   //                                     "0123456789+/";
    
    uint8_t _powerPin;
    uint8_t _resetPin;
  //  unsigned long _timeDelay;
  // private methods
    
    
public:
   
  // public methods

    Sim900(uint8_t receivePin, uint8_t transmitPin, uint8_t powerPin, uint8_t resetPin);
    Sim900(uint8_t receivePin, uint8_t transmitPin);

    ~Sim900();
    
//    void setTimeDelay(unsigned long timeDelay);
    
//    int readResponse(char *buffer, size_t length);
    
    
    size_t readBytesUntil(char *terminator, char *buffer, size_t length);
    size_t readResponse(char *buffer, size_t length);
    //using bool Stream::find(char *);
    //using bool Stream::find(char *, size_t);
    using Stream::find;
    bool find(char *target, size_t length, unsigned long extraTimeout);
    
    // one digit code - 0 for OK, 1 for ERROR, 9 for timeout or wrong response
    int getResponse(void);
    int getURCResponse(char, unsigned long);
    bool accepted(void);
    bool isOn(void);
    
    
    void powerReset(void);
    void powerOn(void);
    void powerToggle(void);
    
    
    //bool gprsOn(void);
    
    // four digits code 11xx
    int gprsOn(const __FlashStringHelper *apn,
                const __FlashStringHelper *user,
                const __FlashStringHelper *pwd);
    
    // four digits code 12xx
    int gprsOff(void);
    
    uint32_t getIP(void);
    
    //bool sendEmail(char *filename, File &file, bool binary = false);
    
    //four digits code 2xxx
    int sendEmail(const __FlashStringHelper *smtp,
                   const __FlashStringHelper *port,
                   const __FlashStringHelper *user,
                   const __FlashStringHelper *pwd,
                   const __FlashStringHelper *emailfrom,
                   const __FlashStringHelper *namefrom,
                   const __FlashStringHelper *emailto,
                   const __FlashStringHelper *nameto,
                   const __FlashStringHelper *subj,
                   const __FlashStringHelper *text,
                   char *filename,
                   File &file,
                   bool binary,
                   unsigned long smtpTimeout);
    
protected:
    
    int base64encode( byte * a3, char * a4, int len = 3);
    
     /*
  void begin(long speed);
  bool listen();
  void end();
  bool isListening() { return this == active_object; }
  bool stopListening();
  bool overflow() { bool ret = _buffer_overflow; if (ret) _buffer_overflow = false; return ret; }
  int peek();
  virtual size_t write(uint8_t byte);
  virtual int read();
  virtual int available();
  virtual void flush();
 */
    
    
//  using Print::write;

};

// Arduino 0012 workaround
/*
#undef int
#undef char
#undef long
#undef byte
#undef float
#undef abs
#undef round
*/
#endif
