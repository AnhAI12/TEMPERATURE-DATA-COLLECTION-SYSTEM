// Writing by Le Duy Anh

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include "stdlib.h"             //random
#include <time.h>               // time_t, struct tm, time, localtime 
#include "MQTTClient.h"         //MQTT
#include <string.h>

//PIN mpu6050
#define INT 11
int8_t mpu6050,temp1,temp2;
volatile int16_t temp=0;
float nd=0.0;               //test

/*-----------MY FUNCTIONS--------*/
void init_mpu6050();
void gia_tri();
void readTemp(uint8_t address);

//test public
float GetRandom(float min,float max){
    return min + (float)(rand()*(max-min+1.0)/(1.0+RAND_MAX));
}

/*------------FUNCTION FOR MQTT PUBLIC------------*/
/*
Broker: broker.emqx.io
TCP Port: 1883 
broker.hivemq.com:1883
*/
#define ADDRESS     "ws://broker.hivemq.com:8000"
#define CLIENTID    "pub_lda"
#define PUB_TOPIC  "plda/iot"

void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = 1;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("Message '%s' with delivery token %d delivered\n", payload, token);
}

/*-----------MAIN FUNCTION-------*/
int main(int argc, char* argv[])
{
    wiringPiSetupPhys();
    pinMode(INT,INPUT);
    wiringPiISR(INT, INT_EDGE_RISING,&gia_tri);     //ham ngat
    //setup i2c
    mpu6050 = wiringPiI2CSetup(0x68);
    ///cau hinh mpu6050
    init_mpu6050();
    ///check ket noi
    if(wiringPiI2CReadReg8(mpu6050,117)==0x68)
    {
        printf("ready\n");
    }
    else printf("not ready\n");

    //MQTT
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;  // return code
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    while (1)
    {
        //read temperature
        readTemp(65);                                           // 65 la thanh ghi nhiet do
        sleep(1);                                               //delay 1s
        //random
        nd = GetRandom(30.0,31.0);
        //get time
        time_t rawtime;
        struct tm *ct;
        time (&rawtime);
        ct = localtime (&rawtime);
        // dd/mm/yyyy hh:mm:ss temp

        //create string time
        char tx_time [20];
        sprintf(tx_time,"%02d:%02d:%02d",ct->tm_hour,ct->tm_min,ct->tm_sec);
        //print
        //printf ("\r %02d:%02d:%02d\n", ct->tm_hour,ct->tm_min,ct->tm_sec);
        //printf("time: %s\n",tx_time);
        readTemp(59);
        temp=((float)temp)/340+36.54;
        //printf("nhiet do: %f\n",((float)temp)/340+36.54);    //tra ve nhiet do
        //printf("%f\n",nd);

        //temp+time
        char msg [40];
        // sprintf(msg,"%2.2f&%s",nd, tx_time);                //test MQTT
        sprintf(msg,"%2.2f&%s",temp, tx_time);
        //send temperature measurement
        publish(client, PUB_TOPIC, msg);
        sleep(0.5);
    }
    
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    return rc;
}

/*--------------- FUNCTIONS------------*/
void gia_tri(){}

void init_mpu6050()
{
    //setup tg lay mau sample, 250HZ
    wiringPiI2CWriteReg8(mpu6050,25,31);
    //config: TURN OFF DLPF
    wiringPiI2CWriteReg8(mpu6050,26,0);
    //GYRO CONFIG +-500 O/S
    wiringPiI2CWriteReg8(mpu6050,27,0x08);
    // ACC CONFI: +-8G
    wiringPiI2CWriteReg8(mpu6050,28,0x10);
    //INT config: enable data ready interrupt
    wiringPiI2CWriteReg8(mpu6050,56,0x01);
    //power config: clock source gyro x
    wiringPiI2CWriteReg8(mpu6050,107,0x01);
}

void readTemp(uint8_t address)
{
    temp1=wiringPiI2CReadReg8(mpu6050,address);
    temp2=wiringPiI2CReadReg8(mpu6050,address+1);
    temp=temp1<<8|temp2;
    // return temp_gt;
}
