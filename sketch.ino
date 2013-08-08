#include <SoftwareSerial.h>

SoftwareSerial cell(7,8);

char inbyte;

// The variables
String msg = String("");
String _ms = String("");
String _msp = String("");
String _sn_tmp = String("");
String _sn_full = String("");
String _en_tmp = String("");
String AuthorizedContact = String("<REDACTED>");


// Pin numbers, iteration values, etc. 
// These can be INTs without an issue.
int _mp = 0;
int in_sms = 0;
int _sn = 0;
int _en = 0;
int iter = 0;

int YouRelay = 10;// YOU        //7
int WillRelay = 11;// WILL      //8
int MarryRelay = 3;
int MeRelay = 4;
int PlayPauseRelay = 6;
int PowerIORelay = 5;

int power_txt = 0;

/*
 *   I know I could use INT's, but they only go from -32,767 to 32,767, as they're 32 bits. 
 *   We're dealing with miliseconds, so there are no negative numbers.
 *   I decicded to use LONGs because of the units being measured. In some cases, such as the one below,
 *   I need a value greater than INTs can handle. 
 *   -AG
 */ 

long _en_full = 0;
long pindelay = 750;
long music_io_delay = 2000;
long music_pp_delay = 250;
long music_wrd_delay = 19000;
long music_strt_delay = 2000;
long music_length = (171000 - (music_pp_delay * 4));

long lightdelay = music_strt_delay + music_wrd_delay - music_pp_delay;

void setup(){

    
    pinMode(7, INPUT);
    pinMode(8, OUTPUT);
    // The communications.
    Serial.begin(19200);
    cell.begin(19200);
    pinMode(WillRelay, OUTPUT);
    pinMode(YouRelay, OUTPUT);
    pinMode(MarryRelay, OUTPUT);
    pinMode(MeRelay, OUTPUT);
    pinMode(PlayPauseRelay, OUTPUT);
    pinMode(PowerIORelay, OUTPUT);

    digitalWrite(WillRelay, HIGH);
    digitalWrite(YouRelay, HIGH);
    digitalWrite(MarryRelay, HIGH);
    digitalWrite(MeRelay, HIGH);
    digitalWrite(PlayPauseRelay, HIGH);
    digitalWrite(PowerIORelay, HIGH);
    Serial.println("MAIN SCREEN TURN ON");
    resetBoard(); 
}

void loop(){

    if(Serial.available()){
        // Look at the incoming byte.
        cell.write((unsigned char)Serial.read());
        }
    else  if(cell.available()){
        // Look at the incoming byte.
        inbyte = (unsigned char)cell.read();
        // If it's a Carrage Return, process it.
        }
        if (inbyte == 13){
            processSerial();
            msg=String(""); 
        }
        // Otherwise append it to the existing buffer.
        if (inbyte == 10){}
        else if (inbyte == -1){}
        else {
            msg +=(inbyte);
       }
    }


 
void processSerial(){

    // If we're successfully connected to the network.
    if (msg.startsWith("Call Ready")){
        Serial.println("[!]Connected to the network!");
        cell.println("AT+CMGF=1");
        delay(500);
    }


    // If an SMS has been received.     
    if (msg.indexOf("+CMTI:") >= 0){
        delay(1000);
        Serial.println("[*]Text message received!");
        Serial.print("[-] RAW: ");
        Serial.println(msg);       
    
        // Extract the message position from the string. 
        _mp = msg.indexOf(",");
        _msp = msg.substring(_mp+1);

        // Read the message
        Serial.println("[-]The Message is at position: "+_msp);
        cell.print("AT+CMGR=");
        cell.println(_msp);
  }

  // The text message command parsing.      
  // This is here intentionally. I kept running into issues with the damned thing looping. 
  if (in_sms == 1){
        msg.toLowerCase();
        Serial.println("[*]Parsing the message");

        if (iter<10){
            // Conditionals...
            if (msg.indexOf("!s") >= 0){
                Serial.println("[!] SYN "+ _sn_full);
                in_sms = 0;
                iter = 0;
                // doSYN();
            } 
            else if(msg.indexOf("!t") >=0){
                Serial.println("[!} Test!");
                doTEST();
                in_sms = 0;
                iter = 0;
            } 
            else if(msg.indexOf("!e") >=0){
                Serial.println("[!] ENGAGE");
                doENGAGE(90000);
                in_sms = 0;
                iter = 0;
            } 
        iter++; 
        } else {
            iter = 0;
            in_sms = 0;
        }
    }
  // Look at the Message.
  if (msg.indexOf("+CMGR: ") >= 0){ 
        Serial.println("[*]Looking at the header info...");   
        cell.print("\r\r");

        // Grab the sending phone number. 
        _sn = msg.indexOf("+1");
        _sn_tmp = msg.substring(_sn+1);
        _sn_full = "";
    
        // -- Look over  the number.. 11 characters... +12025551234, for ex. 
        for (int i=0;i<11;i++){
            _sn_full+=_sn_tmp[i];
        }
    
    
        Serial.println("[-]The sending # is: "+_sn_full);

        // Comparing the sender to the authorized number... 
        if (_sn_full == AuthorizedContact ){
          Serial.println("[-]This is an Authorized Contact!");
//            in_sms = 1;
            doENGAGE(90000);
        }
  }

msg=String("");   
}

void doSYN(){
  power_txt = 1;
  cell.println("AT+CMGS=\""+ AuthorizedContact +"\"");
  delay(1000);
  cell.print("SYN / ACK / SYNACK!");
  delay(1000);
  cell.write(0x1A);
  delay(2000);
  power_txt=0;
}

void doTEST(){
    doWill();
    doYou();
    doMarry();
    doMe();
    resetPins();
}

void doENGAGE(long d){
    mp3IO();
    delay(d);
    musicPP();
    delay(lightdelay);      
    doWill();
    doYou();
    doMarry();
    doMe();
    delay(music_length);
    resetPins();
    mp3IO();     
    resetBoard();
}

void doWill(){
  digitalWrite(WillRelay, LOW);
  delay(pindelay);
}

void doYou(){
    digitalWrite(YouRelay, LOW);
    delay(pindelay);
}

void doMarry(){
    digitalWrite(MarryRelay, LOW);
    delay(pindelay);
}

void doMe(){
    digitalWrite(MeRelay, LOW);
    delay(pindelay);
}

// MP3 Player On/Off
void mp3IO(){
    digitalWrite(PowerIORelay, LOW);
    delay(music_io_delay);
    digitalWrite(PowerIORelay, HIGH);
}   

// Play/Pause Button
void musicPP(){
    digitalWrite(PlayPauseRelay, LOW);
    delay(music_pp_delay);
    digitalWrite(PlayPauseRelay, HIGH);
}

// ResetPins
void resetPins(){
    digitalWrite(WillRelay, HIGH);
    digitalWrite(YouRelay, HIGH);
    digitalWrite(MarryRelay, HIGH);
    digitalWrite(MeRelay, HIGH);
    digitalWrite(PlayPauseRelay, HIGH);
    digitalWrite(PowerIORelay, HIGH);
}

/* Power On/Off the GPRS Board
 * This will likely just be "OFF".
 * Since the board is automatically powered on
 * when everything else is.
 */
void resetBoard(){
  pinMode(9, OUTPUT); 
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(2000);
  digitalWrite(9,LOW);
  delay(3000);
}
