#include <SoftwareSerial.h>

const int trigPin = 2;
const int echoPin = 3;

long duration;
int distance;

boolean started = false;
boolean calling = false;

char temp[150];
int i=1,j=1;
int a;

char url[] = "http://dweet.io/dweet/for/mygps"; // link to which the data has to be posted
char apn[] = "airtelgprs.com";   // your APN

char lat[15];
char lon[15];
char Run[5];
char Fix[5];
char alt[6];
char date[22];
char time[7];
char sat[3];
char spd[10];
char crse[10];

String output = "";
int output_len = 0;

SoftwareSerial module(10, 11); // RX, TX

void setup()
{
  Serial.begin(9600);                           // Open serial communications and wait for port to open:

  module.begin(9600);

  config_cmd( "AT", 1000, 0);                     // Empty Command to GPS
  config_cmd( "AT+CGNSPWR=1", 1000, 0);           // powering up GPS

  module.println("AT+SAPBR=0,1");             // Close Http if its already open
  delay(1000);
  module_write();

  module.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");       // conection establish with network
  delay(2000);
  module_write();

  sprintf(temp, "AT+SAPBR=3,1,\"APN\",\"%s\"", apn);         // APN configure
  module.println(temp);
  delay(2000);
  module_write();

  module.println("AT+SAPBR=1,1");                        //GPRRS connect
  delay(2000);
  module_write();
  delay(2000);

  pinMode(trigPin, OUTPUT);                     // ultrasonic trigger
  pinMode(echoPin, INPUT);                      // reflected signal receiver

}

void loop()
{
  a = ultra_dist();
 Serial.println(a);
 if (a<=10 && a>5)
  {
    if(i==1){
    Serial.println(a);
    dweet();
    i++;
    }
  }
  else if (a<=5 && a>3)
  {
    if(j==1){
    Serial.println(a);
    dweet();
    j++;
    }
  }
  else if(a<=3 && a>0)

  {
     Serial.println(a);
    dweet();
  }
}


void dweet()
{
  output = config_cmd( "AT+CGNSINF", 500, 0); //Get 1 time gps value for length checking and stroe in reposne1
  output_len = output.length();  // to measure the length of the GPS values
  Serial.println("posted");

  if (output_len > 118)
  {
    output = config_cmd( "AT+CGNSINF", 500, 0); //Fetch gps values and store in output

    output.toCharArray(temp, 150);

    strcpy(Run, strtok(temp, ","));
    strcpy(Run, strtok(NULL, ","));
    strcpy(date, strtok(NULL, ","));
    strcpy(lat, strtok(NULL, ","));
    strcpy(lon, strtok(NULL, ","));
    strcpy(alt, strtok(NULL, ","));


    Serial.print("Run=");
    Serial.println(Run);
    Serial.print("date=");
    Serial.println(date);
    Serial.print("Lat=");
    Serial.println(lat);
    Serial.print("Lon=");
    Serial.println(lon);
    Serial.print("alt=");
    Serial.println(alt);
    Serial.flush();
    posting_to_dweet();  // GPRS post after gps is getting data and post is ready.
  }
  else
  {
    if (output_len < 118 && output_len > 15 )
    {
      Serial.println("NO GPS SIGNAL");
      config_cmd( "AT+CGNSINF", 500, 0); //Again fetch if not fetched already
      config_cmd( "AT+CGNSPWR=1", 1000, 0); // power GPS
    }
    else
    {
      Serial.println("NO GPS SIGNAL");
      Serial.flush();
      config_cmd( "AT+CGNSINF", 500, 0); //Again fetch if not fetched already
      config_cmd( "AT+CGNSPWR=1", 1000, 0); // power GPS
    }
  }
  delay(2000);
}

int posting_to_dweet()
{

  module.println("AT+HTTPINIT");
  delay(4000);
  module_write();
  module.println("AT+HTTPPARA=\"CID\",1");
  delay(2000);
  module_write();
  sprintf(temp, "AT+HTTPPARA=\"URL\",\"%s", url);
  module.print(temp);
  delay(150);
  module_write();
  Serial.println(a);
  sprintf(temp, "?visor=false&lat=%s&lon=%s&alt=%s&time=%s&date=%s&sat=%d", lat, lon, alt, time, date, a);
  module.print(temp);
  delay(150);
  module_write();
  module.println("\"");
  delay(2000);
  module_write();
  module.println("AT+HTTPACTION=0");
  delay(5000);
  module_write();
  module.println("AT+HTTPREAD");
  delay(150);
  module_write();
  module.println("AT+HTTPTERM");
  delay(3000);
  module_write();

}

String config_cmd(String command, const int timeout, boolean debug)
{
  String response = "";
  module.println(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (module.available())
    {
      char c = module.read();
      response += c;
    }
  }
  if (debug)
  {
    Serial.println(response);
  }
  return response;
}

int ultra_dist()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  distance = duration * 0.034 / 2;

  //Serial.print("Distance: ");
  //Serial.println(distance);
  return distance;
}

void module_write()
{
  while (module.available() != 0)
    Serial.write(module.read());
}
