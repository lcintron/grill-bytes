#include <Adafruit_DHT_Particle.h>
#include "elapsedtimecheck.h"
#include "stopwatch.h"
#include <Nextion.h>
#define GRILLTHERMISTORPIN A0         
#define MEATTHERMISTORPIN A1       
#define OUTDOORTEMPERATURESENSORPIN D1       
#define OUTDOORTEMPERATURESENSORTYPE DHT22
#define SERIESRESISTOR  10000 
#define THERMISTORNOMINAL 200000      

// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3800


/* NEXTION DISPLAY DEFINITIONS
TYPE    PAGEID  OBJID   OBJNAME     DESCRIPTION
===========================================================================================================================================
page    0       0       splash

page    1       0       main
text    1       3       tempO       Outdoor temperature text
text    1       1       clk         Text of current time (clock)
text    1       5       hum         Text of current humidity percentage in the air
text    1       2       tempG       Text of grill probe temperature
text    1       4       tempM       Text of meat probe temperature
text    1       7       lbl         Text of current grill status (Loading, Ready, Heating, Smoking, BBQ-Roasting, Searing/Baking, Cooling)
text    1       9       stopwatch   Text of current stopwatch time
button  1       8       btnStart    Button that starts/stops stopwatch
button  1       7       btnReset    Button that resets the stopwatch
*/

 
USARTSerial& nexSerial = Serial1;
NexButton  btnStart =NexButton(1,8,"btnStart");
NexButton  btnReset =NexButton(1,7,"btnReset");

NexTouch *nex_listen_list[]={
  &btnStart,
  &btnReset,
  NULL
};


char localIp[] = "xxx.xxx.xxx.xxx";
char pinNumber[] = "x";
String timeStr = "";
String outdoorTemperature = "-";
String outdoorHumidity = "-";
String tempGrill = "-°F";
String tempMeat = "-°F";
ElapsedTimeCheck quarterSecondTimeCheck(250);
ElapsedTimeCheck oneSecondTimeCheck(1000);
ElapsedTimeCheck twoSecondTimeCheck(2000);

String status = "°F";
char buffer[100] = {0};
StopWatch sw;
DHT outdoorTemperatureSensor(OUTDOORTEMPERATURESENSORPIN, OUTDOORTEMPERATURESENSORTYPE);



void setup(){
   nexInit(115200);
   Particle.variable("localIp", localIp);
   Particle.function("sendDisplayCommand", sendDisplayCommandFromCloud);
   outdoorTemperatureSensor.begin();
   Time.zone(-4);
   quarterSecondTimeCheck.start();
   oneSecondTimeCheck.start();
   twoSecondTimeCheck.start();
   getCurrentTime();
   setLocalIp();
   delay(2000);
   sendShowDashboardToDisplay();
   btnStart.attachPop(btnStartPopCallback);
   btnReset.attachPop(btnResetPopCallback);
   sendCurrentTimeToDisplay();
   status = "READY";
}

int sendDisplayCommandFromCloud(String cmd){
    sendDisplayCommand(cmd);
    return 1;
}

void loop(){
    nexLoop(nex_listen_list);
    sw.tick();
    fetchAndUpdateData();
    if(quarterSecondTimeCheck.isTimeElapsed()){
        
        updateDisplay();    
        quarterSecondTimeCheck.reset();
    }
}

void fetchAndUpdateData(){
    if(oneSecondTimeCheck.isTimeElapsed()){
        getCurrentTime();
        oneSecondTimeCheck.reset();
    }
    
    if(twoSecondTimeCheck.isTimeElapsed()){
        getTemperatureHumidity();
        getGrillProbeTemperature();
        getMeatProbeTemperature();
        twoSecondTimeCheck.reset();
    }
}

void updateDisplay(){
    sendCurrentTimeToDisplay();
    sendStopWatchValueToDisplay();
    sendCurrentTempHumToDisplay();
    sendCurrentStatusToDisplay();
    sendMeatProbeTemperatureToDisplay();
    sendGrillProbeTemperatureToDisplay();
}

void btnStartPopCallback(void *ptr){
    sw.toggle();

    if(sw.isActive()){
        sendNumericPropertyValueToDisplay("btnStart", "pic", "2" );
        sendNumericPropertyValueToDisplay("btnStart", "pic2", "2" );
    } else{
        sendNumericPropertyValueToDisplay("btnStart", "pic", "1" );
        sendNumericPropertyValueToDisplay("btnStart", "pic2", "1" );
    }
    
}

void btnResetPopCallback(void *ptr){
    sw.reset();
    sendNumericPropertyValueToDisplay("btnStart", "pic", "1" );
    sendNumericPropertyValueToDisplay("btnStart", "pic2", "1" );
}

String getMeatProbeTemperature(){
    int temp = getProbeTemperature(MEATTHERMISTORPIN);
    tempMeat = temp == -1000? "-°F":String(celsiusToF(temp))+"°F";
    return tempMeat;
}

String getGrillProbeTemperature(){
    int temp = getProbeTemperature(GRILLTHERMISTORPIN);
    tempGrill = temp == -1000? "-°F":String(celsiusToF(temp))+"°F";
    return tempGrill;
}

int getProbeTemperature(int probePin){
  uint8_t i;
  float average;
  int samples[NUMSAMPLES];
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(probePin);
   delay(10);
  }
  
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  if( average >= 4093.0){
        return -1000;
  }
  
  
  // convert the value to resistance
  average = 4095 / average - 1;
  average = SERIESRESISTOR / average;
 
  float steinhart;
  
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;   
  
  return steinhart;
}



void setLocalIp(){
    IPAddress myAddr = WiFi.localIP();
    byte first_octet = myAddr[0];
    byte second_octet = myAddr[1];
    byte third_octet = myAddr[2];
    byte fourth_octet = myAddr[3];
    sprintf(localIp, "%d.%d.%d.%d", first_octet, second_octet, third_octet, fourth_octet);
}

void sendTextValueToDisplay(String objName, String txt){
    sendDisplayCommand(objName+".txt="+"\""+txt.c_str()+"\"");
}

void sendNumericPropertyValueToDisplay(String objName, String prop, String value){
    sendDisplayCommand(objName+"."+prop+"="+value);
}

void sendDisplayCommand(String cmd){
    Serial1.print(cmd);
    Serial1.write(0xff);  
    Serial1.write(0xff);
    Serial1.write(0xff);
}

void sendMeatProbeTemperatureToDisplay(){
     sendTextValueToDisplay("tempM",tempMeat);
}

void sendGrillProbeTemperatureToDisplay(){
     sendTextValueToDisplay("tempG",tempGrill);
}

void sendCurrentTimeToDisplay(){
    sendTextValueToDisplay("clk", timeStr);

}    

void sendShowDashboardToDisplay(){
    sendDisplayCommand("page main");
}

void sendStopWatchValueToDisplay(){
    sendTextValueToDisplay("stopwatch", sw.toString());
}

void sendCurrentStatusToDisplay(){
     sendTextValueToDisplay("lbl",status);
}

void sendCurrentTempHumToDisplay(){
    sendTextValueToDisplay("tempO", outdoorTemperature+"°F");
    sendTextValueToDisplay("hum", outdoorHumidity + "%");
}


int celsiusToF(float celsius){
    return (int)round(1.8*(celsius)+32);
}

int getTemperatureHumidityReading(String cmd){
    getTemperatureHumidity();
    return 1;
}

void getTemperatureHumidity(){
    
    float h = outdoorTemperatureSensor.getHumidity();
	float f = outdoorTemperatureSensor.getTempFarenheit();
  
	if (isnan(h) ||  isnan(f)) {
	    Particle.publish("Temperature", "ERROR");
	    return;
	}
	
	outdoorTemperature = String((int)f);
    outdoorHumidity = String((int)h);     
    
}

void getCurrentTime(){
    int hour = Time.hour();
    int minute = Time.minute();
    String hourString =String(hour);
    String minuteString = String(minute);
    if(hour <10 )
        hourString = "0"+String(hour);
    
    if(minute <10)
        minuteString = "0"+String(minute);
    timeStr =hourString+":"+minuteString;
}