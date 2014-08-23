#include <SPI.h>
#include <Ethernet.h>
#include <aJSON.h>
 
#define RELEPIN 7 //porta conectada ao Relé
static char apiKey[] = "xxxxxxxx-yyyy-zzzz-wwww-kkkkkkkkkkkk"; //@todo Alterar para a API Key do Projeto.
static char serial[] = "GUC0002"; //@todo Alterar para a Serial do Dispositivo.
 
unsigned long frequencyTime = 1000; //Definicao do tempo que devera ser lido o estado do atuador.
static byte mac[] = { 
  0xDE, 0xAD, 0x09, 0xEF, 0xFE, 0xED };
static char serverName[] = "guardiao.cl";
 
/**
//Caso queira definir o IP manualmente, descomente.
IPAddress ip(10,10,1,10);
IPAddress dnsserver(10,10,1,1);
IPAddress gateway(10,10,1,1);
IPAddress subnet(255,255,255,0);
**/
 
String jsonString = "";
EthernetClient client;
 
void setup(){
  Serial.begin(9600);
 
  pinMode(RELEPIN,OUTPUT);
  digitalWrite(RELEPIN,LOW);
  Ethernet.begin(mac); //IP Automático 
  //Ethernet.begin(mac, ip, dnsserver, gateway, subnet); //IP Manual
  Serial.print(F("IP Atual: "));
  Serial.println(Ethernet.localIP());
}
 
void loop(){
  char inChar;
  boolean beginJson = false;
  char outBuf[128];
  char host[64];
 
  if(client.connect(serverName, 80)) {
    sprintf(outBuf,"GET /actuator/%s/?apiKey=%s HTTP/1.1",serial,apiKey);
    client.println(outBuf);
    sprintf(host,"Host: %s",serverName);
    client.println(host);
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();
 
    Serial.println("Iniciando leitura JSON");
    while(client.connected()){
      while(client.available()){
        inChar = client.read();
        if(inChar=='{'){
          beginJson = true;
        }
        if(beginJson){
          jsonString += inChar;
        }
      }
    }
    if(jsonString!=""){
      char jsonChar[jsonString.length()];
      jsonString.toCharArray(jsonChar, jsonString.length() + 1);
      aJsonObject* root = aJson.parse(jsonChar);
      aJsonObject* situacao = aJson.getObjectItem(root, "state"); 
      Serial.print("Estado: ");
      if(situacao->valuebool)
      {
        Serial.println("Ativo"); 
      }else
      {
        Serial.println("Desativado"); 
      }
 
      if(situacao->valuebool){
        digitalWrite(RELEPIN,HIGH);
      }
      else{
        digitalWrite(RELEPIN,LOW);
      }
 
      aJson.deleteItem(root);
    }  
  } 
  else {
    Serial.println("Falha de conexao");
  }
 
  if (!client.connected()) {
    client.stop();
  }
 
  jsonString = "";
 
  delay(frequencyTime);
}