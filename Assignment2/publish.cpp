// Based on the Paho C code example from www.eclipse.org/paho/
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include "MQTTClient.h"
#define  CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"
#include "ADXL345.h"
#include <unistd.h>
#include <signal.h>
using namespace std;
using namespace exploringRPi;
//Please replace the following address with the address of your server
#define ADDRESS    "tcp://192.168.45.220:1883"
#define CLIENTID   "rpi1"
#define AUTHMETHOD "siddharth"
#define AUTHTOKEN  "pavithra@01"
#define TOPIC      "ee513/CPUTemp"
#define QOS        1

#define TIMEOUT    500L

float getCPUTemperature() {        // get the CPU temperature
   int cpuTemp;                    // store as an int
   fstream fs;
   fs.open(CPU_TEMP, fstream::in); // read from the file
   fs >> cpuTemp;
   fs.close();
   return (((float)cpuTemp)/1000);
}




int main(int argc, char* argv[]) {
   string lastWillMessage = "{\"d\":{\"CPUTemp\": -1, \"Pitch\": -1, \"Roll\": -1, \"LastWill\": \"Publisher is disconnected.\" }}";
   char str_payload[100];
   float pitch;
   float roll;
   ADXL345 sensor(1,0x53);
   MQTTClient client;
   MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
   MQTTClient_message pubmsg = MQTTClient_message_initializer;
   MQTTClient_deliveryToken token;
   MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
   opts.keepAliveInterval = 20;
  opts.cleansession = 1;
   opts.username = AUTHMETHOD;
   opts.password = AUTHTOKEN;
   sensor.setResolution(ADXL345::NORMAL);
   sensor.setRange(ADXL345::PLUSMINUS_4_G);
  
  
   int rc;
   if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
      cout << "Failed to connect, return code " << rc << endl;
      return -1;
   }

    MQTTClient_willOptions willOptions = MQTTClient_willOptions_initializer;
    willOptions.message = lastWillMessage.c_str();
    willOptions.topicName = TOPIC;
    willOptions.qos = QOS;
    willOptions.retained = 0;
    opts.will = &willOptions;
   while(true)
{
   sensor.displayPitchAndRoll(1,pitch,roll);
   sprintf(str_payload, "{\"CPUTemp\": %f, \"Pitch\": %f, \"Roll\": %f }", getCPUTemperature(), pitch, roll);
   pubmsg.payload = str_payload;
   pubmsg.payloadlen = strlen(str_payload);
   pubmsg.qos = QOS;
   pubmsg.retained = 0;
   MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << str_payload <<
        " \non topic " << TOPIC << " for ClientID: " << CLIENTID << endl;
   rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
   cout << "Message with token " << (int)token << " delivered." << endl;
   usleep(1000000);
 //MQTTClient_setCallbacks(client,NULL,NULL,onConnectionLost,NULL);
 }
   
   MQTTClient_destroy(&client);
   return 0;

}
