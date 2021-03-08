# WiFi Manager
The function of the WiFi manager is to help the user connect to a WiFi network. If no known network is found the ESP8266 will start a hotspot with a standalone portal in which the network settings can be changed. WiFi information will be preserved in memory so that the ESP8266 will connect automatically in the future.

## Class Methods

#### begin

```c++
void begin(char const *apName);
```
This method must be called from the setup of the application. The mandatory argument is the SSID name that will be used as access point name nad as localhost. The WiFi manager will connect to the stored WiFi details. If no details are stored, a standalone portal will be started from 192.168.4.1. 

#### loop

```c++
void loop();
```
This method must be called from the main loop of the application and allows to set and change the wifi details asynchronously from the web server call.

#### prepareWiFi_STA_forget

```c++
void prepareWiFi_STA_forget();
```
A call to this function will force forgetWiFi the stored WiFi details and start a standalone portal.

#### isApMode

```c++
bool isApMode();
```
Returns true if a standalone portal is currently active.

#### getSSID()

```c++
String getSSID();
```
Returns the SSID to which the ESP8266 is connected.
Returns an empty string if a WiFi is not connected.

#### getLocalIP();

```c++
static String getLocalIP();
```
Returns the local IP of the ESP8266.
Returns an empty string if a WiFi is not connected.

#### getSubnetMask();

```c++
static String getSubnetMask();
```

Returns the subnet mask of the ESP8266.
Returns an empty string if a WiFi is not connected.

#### getGatewayIP();

```c++

static String getGatewayIP();
```
Returns the gateway IP of the ESP8266.
Returns an empty string if a WiFi is not connected.

#### getDnsIP();

```c++

static String getDnsIP();
```

Returns the DNS IP of the ESP8266.
Returns an empty string if a WiFi is not connected.

#### isDHCP();

```c++

static bool isDHCP();
```

Returns `true` if WiFi is connected and ESP8266 uses external DHCP.

#### prepareWiFi_AP_PSK(String newPass);

```c++

void prepareWiFi_AP_PSK(String newPass);
```

Forces change of access point password.
Password has to be at least 8 characters
If password is empty: open (not secured) access point will be started

#### prepareWiFi_STA()

```c++
void prepareWiFi_STA(String newSSID, String newPass);
void prepareWiFi_STA(String newSSID, String newPass, const String &newLocalIP, const String &newSubnetMask, const String &newGatewayIP, const String &newDnsIP);
```
Tries to connect to the WiFi network with getSSID `newSSID` and password `newPass`. If this fails a reconnect to the known network will be attempted. If this also fails or if no previous network was known, a standalone portal will be started. Alternatively the function can also be called with inputs for a static IP address if DHCP is not available.

## Web interface

The page in the web interface that is connected to the WiFi settings is shown below. For now this is a simple page that:
* shows the currently connected network
* allows you to prepareWiFi_STA_forget the current WiFi details
* allows you to set a new getSSID and password.
* allows you to set a static IP address.

![](https://raw.githubusercontent.com/maakbaas/esp8266-iot-framework/master/docs/img/screenshot-wifi.png)
