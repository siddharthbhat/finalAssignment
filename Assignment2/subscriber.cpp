#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>
#include <gpiod.hpp>
#include <unistd.h>

#define LED_PIN 27
#define LED_PIN2 22
#define ADDRESS   "tcp://192.168.45.220:1883"
#define CLIENTID  "rpi2"
#define AUTHMETHOD "siddharth"
#define AUTHTOKEN "pavithra@01"
#define TOPIC    "ee513/CPUTemp"
#define PAYLOAD   "Hello World!"
#define QOS     1
#define TIMEOUT   10000L
 struct gpiod_chip *chip;
 struct gpiod_line *line;
volatile MQTTClient_deliveryToken deliveredtoken;
int messagecnt = 0;
bool messageReceived = false;
void messageled()
{
 chip = gpiod_chip_open("/dev/gpiochip0");
 line = gpiod_chip_get_line(chip, LED_PIN2);
 gpiod_line_request_output(line, "LED blink", 1);
 gpiod_line_set_value(line, 0);
 printf("led on");
 sleep(500000);
 gpiod_line_request_output(line, "LED blink", 1);
 gpiod_line_set_value(line, 1);
 gpiod_chip_close(chip);
 printf("led off");

}
void turnonled() {

  int value = 1;

  chip = gpiod_chip_open("/dev/gpiochip0");
  line = gpiod_chip_get_line(chip, LED_PIN);
  gpiod_line_request_output(line, "LED blink", 1);
  gpiod_line_set_value(line, value);
 printf("LED is on\n");
  sleep(2);
  value=0;
  gpiod_line_request_output(line, "LED blink", 1);
  gpiod_line_set_value(line, value);
  gpiod_chip_close(chip);
  printf("LED is off\n");
}

void delivered(void *context, MQTTClient_deliveryToken dt) {
  printf("Message with token value %d delivery confirmed\n", dt);
  deliveredtoken = dt;
}

void *timer_thread(void *arg) {
  while(1)
{

  sleep(4); // Sleep for 4 seconds

  if(!messageReceived){
  // Turn off the LED
  struct gpiod_chip *chip;
  struct gpiod_line *line;
  chip = gpiod_chip_open("/dev/gpiochip0");
  line = gpiod_chip_get_line(chip, LED_PIN2);
  gpiod_line_request_output(line, "LED blink", 1);
  gpiod_line_set_value(line, 0);
  gpiod_chip_close(chip);
}
  messageReceived = false;
}
  return NULL;
}


int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
  messagecnt++;
  int i;
  struct gpiod_chip *chip;
  struct gpiod_line *line;
  char *payloadptr;
 printf("Message arrived\n");
  printf("   topic: %s\n", topicName);
  printf("  message: ");
 payloadptr = (char *)message->payload;
  for (i = 0; i < message->payloadlen; i++) {
    putchar(*payloadptr++);
  }
  putchar('\n');
  if (messagecnt == 10) {
     turnonled();
  }
  messageReceived = true;
  chip = gpiod_chip_open("/dev/gpiochip0");
  line = gpiod_chip_get_line(chip, LED_PIN2);
  gpiod_line_request_output(line, "LED blink", 1);
  gpiod_line_set_value(line, 1); // Turn on the LED
  printf("LED is on\n");
  MQTTClient_freeMessage(&message);
  MQTTClient_free(topicName);
  return 1;
}

void connlost(void *context, char *cause) {
  printf("\nConnection lost\n");
  printf("   cause: %s\n", cause);
}

int main(int argc, char *argv[]) {
  MQTTClient client;
  MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
  int rc;
  int ch;
  pthread_t timer_thread_id;
  pthread_create(&timer_thread_id, NULL, timer_thread, NULL);

  MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);
  opts.keepAliveInterval = 20;
  opts.cleansession = 1;
  opts.username = AUTHMETHOD;
  opts.password = AUTHTOKEN;

  MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
  if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
    printf("Failed to connect, return code %d\n", rc);
 }

  printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
 "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);

  MQTTClient_subscribe(client, TOPIC, QOS);

  do {
    ch = getchar();
  } while (ch != 'Q' && ch != 'q');
  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);
  return rc;
}
