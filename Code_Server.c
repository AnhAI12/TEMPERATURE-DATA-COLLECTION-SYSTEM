#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "MQTTClient.h"
#include <stdint.h>
#include </usr/include/mariadb/mysql.h>

volatile float tempC;
uint8_t cnt;

/* broker.hivemq.com:1883*/
/*tcp://broker.emqx.io:1883*/
#define ADDRESS     "ws://broker.hivemq.com:8000"
#define CLIENTID    "sub_lda"
#define SUB_TOPIC   "plda/iot"
// #define PUB_TOPIC   "test/topic2"
// #define QOS         1

MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;

char *server = "localhost";
char *user = "userpc";
char *password = "18"; /* set me first */
char *database = "lda_IOT";

char tx_time[20];
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = message->payload;
    printf("Received message: %s\n", payload);
    sscanf(payload, "%f&%s", &tempC, tx_time);
    cnt++;

    //printf("Temperature C: %2.2f & Time: %s\n", tempC, tx_time);
    char datasql[200];

    conn = mysql_init(NULL);
    if (mysql_real_connect(conn, server, user, password, database, 0, NULL, 0) == NULL) 
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }  	

    sprintf(datasql,"insert into test_wt(temp_db,tx_time) values (%f,'%s')",tempC,tx_time); //mai sua ,tempC,tx_time20.0,'11:11:11'
    mysql_query(conn,datasql);
    mysql_close(conn);
   
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

int main(int argc, char* argv[]) {
    /////////////////////////// MQTT /////////////
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    //conn_opts.username = "your_username>>";
    //conn_opts.password = "password";

    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);

    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
   
    //listen for operation
    MQTTClient_subscribe(client, SUB_TOPIC, 0);

    ////////////////////////// MySQL ///////////
    cnt = 0;
    while(1) {
        //send temperature measurement
        // publish(client, PUB_TOPIC, "HELLO WORLD!");
        // sleep(3);
        // conn = mysql_init(NULL);
        // mysql_real_connect(conn,server,user,password,database,0,NULL,0); 

    }
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    return rc;
}