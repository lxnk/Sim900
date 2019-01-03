/*
Sim900.cpp -
DESCRIPTION
*/

// Some pins/other definitions

#define POWER_PIN           8 // connect GSM Module POWER to pin 8
#define RESET_PIN           9 // connect GSM Module RESET to pin 9


#define GPRS_TIME_DELAY     300
#define GPRS_TIMEOUT        30000

//
// Includes
//

//#include <avr/interrupt.h>
//#include <avr/pgmspace.h>
//#include <Arduino.h>
#include <SoftwareSerial.h>
//#include <util/delay_basic.h>
#include <SPI.h>
#include <SD.h>
#include <Sim900.h>



/***********************************
 
 Check online
 
 AT0
 
 Turning GPRS on
 
 AT+THECOMMAND=P1,P2,...\r
 
 AT+
    SAPBR=
          3,1,
              "CONTYPE","GPRS"
              "APN","internet.t-mobile"   // Example for Deutsche Telekom network
              "USER","t-mobile"
              "PWD","tm"
          1,1
 
 Sending email
 
 AT+
    EMAIL
         SSL=1
         CID=1
         TO=30
    SMTP
        SRV="smtp.server.name",465  // SSL
                               587  // TSL
        AUTH=1,"login","password"
        FROM="myemail@server.name","My Name"
        RCPT=0,0,"recipientemail@server.name","Recipient Name"
        SUB="Subject line"
        BODY
Body of the email

with new lines.\nMustend with\x1A
        FILE=2,\"filename.ext\",1\r
 
 
 */




const char PROGMEM b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789+/";

//
// Private methods
//

//
// Public methods
//

//
// Constructor
//

Sim900::Sim900(uint8_t receivePin, uint8_t transmitPin, uint8_t powerPin, uint8_t resetPin) : SoftwareSerial(receivePin, transmitPin)
{
    _powerPin = powerPin;
    _resetPin = resetPin;
};


Sim900::Sim900(uint8_t receivePin, uint8_t transmitPin) : SoftwareSerial(receivePin, transmitPin)
{
    _powerPin = POWER_PIN;
    _resetPin = RESET_PIN;
};


//
// Destructor
//

Sim900::~Sim900()
{
    end();
}

/*
void Sim900::setTimeDelay(unsigned long timeDelay)  // sets the maximum number of milliseconds to wait
{
    _timeDelay = timeDelay;
}
*/


size_t Sim900::readBytesUntil(char *terminator, char *buffer, size_t length)
{
    if (length < 1) return 0;
    size_t index = 0;
    Stream::MultiTarget t = {terminator, strlen(terminator), 0};    // the target that will be used for parsing
    
    while (index < length) {       // run as long as buffer is not full
        int c = timedRead();       // read byte
        
        if ( c != t.str[t.index] ) {
            for (int i = 0; i < t.index; i++) {      // dump the matching part of the terminator string
                *buffer++ = t.str[i];
            }
            index += t.index;
            t.index = 0;
            
            if (c < 0) break;      // in case of reading error stop
            
            if ( c!= t.str[t.index] ) {
                *buffer++ = (char)c;   // write normal byte
                index++;
            } else {
                t.index++;         // it could be the start of the sequence
            }
        } else {
            t.index++;             // switch to next character in terminator string
            if (t.index == t.len) break; // stop if all string matched
        }
    }
    
    return index; // return number of characters, not including the terminator length
}


// Search subroutine that shows whether the command was accepted
// Looks for <CR><LF>OK<CR><LF>, if find <CR><LF>ERROR<CR><LF> - quits with error -1
// One digit code - 0 for OK, 1 for ERROR, 9 for timeout or wrong response
int Sim900::getResponse(void)
{
    MultiTarget t[2] = {{"\r\nOK\r\n", 6, 0}, {"\r\nERROR\r\n", 9, 0}};
    return -(B111 & findMulti(t, 2));
}


// For correct processing of the response +SMTPSEND: 1
// one has to put c=0 (string termination) i.e. call getURCResponse(0, ...)

int Sim900::getURCResponse(char c, unsigned long extraTimeout)
{
    int res;
    char str[] = "\r\n+SMTPFT:  ,";
    str[11] = c;
    //char cc;
    MultiTarget t[2] = {{str, 13, 0}, {"\r\n+SMTPSEND: ", 13, 0}};
    
    _timeout += extraTimeout;
    
    res = B111 & findMulti(t, 2);
   
    _timeout -= extraTimeout;
    
    switch (res) {
        case 0:
            res = parseInt();
            if (!find("\r\n", 2)) return -5;
            else return res;
        case 1:
            res = parseInt();
            if (!find("\r\n", 2)) return -6;
            if ( c == 0 && res == 1) return 0;
            else return -res;
        default:
            return -res;
    }
}

bool Sim900::accepted(void)
{
    return getResponse() == 0 ? true : false;
}

bool Sim900::isOn(void)
{
    print(F("ATE0\r"));
    return accepted();
}

bool Sim900::find(char *target, size_t length, unsigned long extraTimeout)
{
    bool res;
    
    _timeout += extraTimeout;
    res = find(target, length);
    _timeout -= extraTimeout;
    
    return res;
}

size_t Sim900::readResponse(char *buffer, size_t length)
{
    if (!find("\r\n")) return 0;
    return readBytesUntil("\r\n", buffer, length);
}




int Sim900::base64encode( byte* a3, char* a4, int len)
{
    int i = 0;
    
    for(i = len; i < 3; i++) a3[i] = 0;
    
    a4[0] = (a3[0] & 0xfc) >> 2;
    a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
    a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
    a4[3] = (a3[2] & 0x3f);
    
    for(i = 0; i < 4; i++)
    {
        a4[i] = pgm_read_byte(&b64_alphabet[a4[i]]);
    }
    
    for(i = len + 1; i < 4; i++) a4[i] = '=';
    
    return len;
}


// Function powers the GPRS Shield by pulling a pin GSM_RESET

void Sim900::powerToggle(void)
{
    // set POWER pin as output
    pinMode(_powerPin, OUTPUT);
    
    // generate power pulse which can both turn on and shut down
    
    digitalWrite(_powerPin, HIGH);
    delay(1100);
    digitalWrite(_powerPin, LOW);
    delay(2300);
}

void Sim900::powerOn(void)
{
    // set POWER pin as output
    pinMode(_powerPin, OUTPUT);
    
    // generate short turn on pulse - it is too short to shut down
    
    digitalWrite(_powerPin, HIGH);
    delay(500);
    digitalWrite(_powerPin, LOW);
    delay(2300);
}

// Function resets the GPRS Shield by pulling a pin GSM_RESET

void Sim900::powerReset(void)
{
    // set RESET pin as output
    pinMode(_resetPin, OUTPUT);
    
    // generate reset pulse
    
    digitalWrite(_resetPin, HIGH);
    delay(50);
    digitalWrite(_resetPin, LOW);
    delay(1300);
}



// four digits code 11xx

int Sim900::gprsOn(const __FlashStringHelper *apn,
                   const __FlashStringHelper *user,
                   const __FlashStringHelper *pwd)
{
    unsigned long startTime;
    int rCode = 0;
    
    print(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r"));
    rCode = getResponse();
    if (rCode) return -1100 + rCode;
    
    print(F("AT+SAPBR=3,1,\"APN\",\""));
    print(apn);
    print(F("\"\r"));
    rCode = getResponse();
    if (rCode) return -1110 + rCode;
    
    print(F("AT+SAPBR=3,1,\"USER\",\""));
    print(user);
    print(F("\"\r"));
    rCode = getResponse();
    if (rCode) return -1120 + rCode;
    
    print(F("AT+SAPBR=3,1,\"PWD\",\""));
    print(pwd);
    print(F("\"\r"));
    rCode = getResponse();
    if (rCode) return -1130 + rCode;
    
    
    startTime = millis();
    rCode = 0;
    do {
        if (millis() - startTime > GPRS_TIMEOUT) return -1140 + rCode;
        delay(GPRS_TIME_DELAY);
        print(F("AT+SAPBR=1,1\r"));
        rCode = getResponse();
    } while (rCode);
        
    return 0;
}


// four digits code 12xx

int Sim900::gprsOff(void)
{
    int rCode = 0;
    print(F("AT+SAPBR=0,1\r"));
    rCode = getResponse();
    if (rCode) return -1200 + rCode;
    return 0;
}


uint32_t Sim900::getIP(void)
{
    uint32_t ip = 0;
    
    print(F("AT+SAPBR=2,1\r"));
    if (!find("\r\n+SAPBR: 1,1,\"", 15, GPRS_TIMEOUT)) return 0;
    do {
        ip = parseInt();
        ip << 8;
    } while ( '.' != timedRead() );
    if (!find("\r\n")) return 0;
    
    return ip;
}



// four digits code 2xxx

int Sim900::sendEmail(const __FlashStringHelper *smtp,
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
                       unsigned long smtpTimeout)
{
    size_t blockSize;
    size_t bytesLeft;
    size_t bufLength;
    byte bBuf[3];
    char tBuf[4];
    int rCode = 0;
    //File file;
    
    // DEBUG
    //Serial.print(F("Preparing email: "));
   
    print(F("AT+EMAILSSL=1\r"));
    rCode = getResponse();
    if (rCode) return -2000 + rCode;

    print(F("AT+EMAILCID=1\r"));
    rCode = getResponse();
    if (rCode) return -2010 + rCode;
    
    print(F("AT+EMAILTO="));
    print((int)(smtpTimeout/1000));
    print(F("\r"));
    rCode = getResponse();
    if (rCode) return -2020 + rCode;
    
    print(F("AT+SMTPSRV=\""));
    print(smtp);
    print(F("\","));
    print(port);
    print(F("\r"));
    rCode = getResponse();
    if (rCode) return -2030 + rCode;
    
    print(F("AT+SMTPAUTH=1,\""));
    print(user);
    print(F("\",\""));
    print(pwd);
    print(F("\"\r"));
    rCode = getResponse();
    if (rCode) return -2040 + rCode;
    
    print(F("AT+SMTPFROM=\""));
    print(emailfrom);
    print(F("\",\""));
    print(namefrom);
    print(F("\"\r"));
    rCode = getResponse();
    if (rCode) return -2050 + rCode;
    
    print(F("AT+SMTPRCPT=0,0,\""));
    print(emailto);
    print(F("\",\""));
    print(nameto);
    print(F("\"\r"));
    rCode = getResponse();
    if (rCode) return -2060 + rCode;
    
    print(F("AT+SMTPSUB=\""));
    print(subj);
    print(F("\"\r"));
    rCode = getResponse();
    if (rCode) return -2070 + rCode;
    
    print(F("AT+SMTPBODY\r"));
    if (!find("\r\n> ")) return 2080;
    print(text);
    print(F("\x1A"));
    rCode = getResponse();
    if (rCode) return -2080 + rCode;
    
    
    
    
    // DEBUG
    //Serial.println(F("Mail configured."));
    
    if (binary) {
        print(F("AT+SMTPFILE=2,\""));
        print(filename);
        print(F("\",1\r"));
        
        // DEBUG
        //Serial.print(F("Binary file "));
    } else {
        print(F("AT+SMTPFILE=1,\""));
        print(filename);
        print(F("\",0\r"));
        
        // DEBUG
        //Serial.print(F("Text file "));
    }
    rCode = getResponse();
    if (rCode) return -2090 + rCode;
    
    // DEBUG
    //Serial.println(F("set."));
    
    
    //file = SD.open(filename, FILE_READ);
    if (!file) {
        // DEBUG
        //Serial.println(F("File opening failed."));
        return -2100;
    }
    
    // DEBUG
    //Serial.println(F("File opened."));
    
    print(F("AT+SMTPSEND\r"));
    rCode = getResponse();
    if (rCode) return -2200 + rCode;
    
    // DEBUG
    //Serial.println(F("SMTPSEND accepted."));
    /*
    _timeout +=20000;
    //Serial.write(timedRead());
    while (true) {
        Serial.write(timedRead());
    }
    _timeout -=20000;
    Serial.println(F("Done"));
    return false;
    */
    
    /*
     *  Here one should expect as \r\n+SMTPFT: 1,x so \r\n+SMTPSEND: yy
     */
    
    rCode = getURCResponse('1', smtpTimeout);
    
    // DEBUG
    //Serial.print(F("rCode = "));
    //Serial.println(rCode);
    
    if (rCode < 0) {
        // DEBUG
        //Serial.print(F("Fail "));
        //Serial.println(-3100 + rCode);
        
        return -3100 + rCode;
    }
    blockSize = rCode;
    
    //if (!find("\r\n+SMTPFT: 1,", 13, smtpTimeout)) return false;
    //blockSize = parseInt();
    //if (!find("\r\n", 2)) return false;
    
    // DEBUG
    //Serial.println(F("File request received."));
    //Serial.print(F("blockSize = "));
    //Serial.println(blockSize);
    
    
    // DEBUG
    //Serial.print(F("Size of file: "));
    //Serial.print(file.size());
    //Serial.print(F(", bytes available: "));
    //Serial.println(file.available());
    
    while (bytesLeft = file.available()) {
        
        // Cut for base 64
        // blockSize is the length of the block after base64 encoding
        // bytesLeft is number of non-encoded bytes in the file

        if (binary)
        {
            blockSize = blockSize / 4 * 4;
            if (blockSize / 4 * 3 > bytesLeft)
            {
                blockSize = bytesLeft / 3 * 4 + ((bytesLeft % 3) ? 4 : 0);
            }
        } else {
            if (blockSize > bytesLeft) blockSize = bytesLeft;
        }
        
        // DEBUG
        //Serial.println(F("Size adapted:"));
        //Serial.print(F("blockSize = "));
        //Serial.print(blockSize);
        //Serial.print(F(", bytesLeft = "));
        //Serial.println(bytesLeft);
        
        print(F("AT+SMTPFT="));
        print(blockSize, DEC);
        print('\r');
        
        // DEBUG
        //Serial.println(F("AT+SMTPFT sent."));
        
        rCode = getURCResponse('2', smtpTimeout);
        if (rCode < 0) return -3200 + rCode;
        if (blockSize != rCode) return -3200;
        
        //if (!find("\r\n+SMTPFT: 2,", 13)) return false;
        //if (blockSize != parseInt()) return false;
        //if (!find("\r\n",2)) return false;
        
        // DEBUG
        //Serial.println(F("AT+SMTPFT approved."));

        
        if (binary) {
            // DEBUG
            //Serial.println(F("Ready to read, encode, and send binary bytes."));
            
            while (blockSize) {
                bufLength = (bytesLeft > 2 ) ? 3 : bytesLeft;
                file.readBytes(bBuf, bufLength);
                base64encode(bBuf, tBuf, bufLength);
                write(tBuf, 4);
                blockSize -= 4;
                bytesLeft -= bufLength;
            }
            
        } else {
            // DEBUG
            //Serial.println(F("Copy ASCII bytes."));
            
            while (blockSize) {
                write(file.read());
                blockSize--;
                bytesLeft--;
            }
        }
        
        // DEBUG
        //Serial.println(F("Data dumped."));
        //Serial.print(F("blockSize = "));
        //Serial.print(blockSize);
        //Serial.print(F(", bytesLeft = "));
        //Serial.println(bytesLeft);
        
        
        rCode = getResponse();
        if (rCode) return -3300 + rCode;
        
        // DEBUG
        //Serial.println(F("Data accepted."));
        
        rCode = getURCResponse('1', smtpTimeout);
        if (rCode < 0) return -3100 + rCode;
        blockSize = rCode;
        
        //if (!find("\r\n+SMTPFT: 1,", 13, smtpTimeout)) return false;
        //blockSize = parseInt();
        //if (!find("\r\n",2)) return false;
        
        // DEBUG
        //Serial.println(F("New file request received."));
        //Serial.print(F("blockSize = "));
        //Serial.println(blockSize);
        
    }
    
    // DEBUG
    //Serial.print(F("Size of file: "));
    //Serial.print(file.size());
    //Serial.print(F(", bytes available: "));
    //Serial.println(file.available());
    
    // Is it really required?
    file.flush();
    
    // DEBUG
    //Serial.println(F("File closed."));
    
    print(F("AT+SMTPFT=0\r"));
    rCode = getResponse();
    if (rCode) return -4000 + rCode;
    
    // DEBUG
    //Serial.println(F("Data transfer closed."));
    
    rCode = getURCResponse(0, smtpTimeout);
    if (rCode) return -4100 + rCode;
    //if (!find("\r\n+SMTPSEND: 1\r\n", 16, smtpTimeout)) return false;
    
    // DEBUG
    //Serial.println(F("Sending mail confirmed."));
    
    return 0;
}


