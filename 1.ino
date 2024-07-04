/********************************************
  文件名：40.WIFI Web Servers LED.ino
  版本：V2.0
  author: zhulin
  说明：WIFI Web Servers 双色LED灯实验
 ********************************************/
#include <WiFi.h>

// 输入自己可以上网的WIFI名称和密码
const char* ssid = "iPhone LK (2)";
const char* password = "11223344";

// 设置web服务器端口号为80
WiFiServer server(80);

// 变量来存储HTTP请求
String header;

// 用于存储当前输出状态的辅助变量
String output25State = "off";
String output26State = "off";

// 给GPIO管脚分配输出变量
const int output25 = 25;
const int output26 = 26;

// 当前时间
unsigned long currentTime = millis();
// 之前的时间
unsigned long previousTime = 0; 
// 定义超时时间(以毫秒为单位)(例如:2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // 将输出变量初始化为输出
  pinMode(output25, OUTPUT);
  pinMode(output26, OUTPUT);
  // 将输出设置为LOW
  digitalWrite(output25, LOW);
  digitalWrite(output26, LOW);

  // 使用SSID和密码连接Wi-Fi网络
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // 打印本地IP地址并启动web服务器
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // 监听 clients

  if (client) {                             // 如果新客户端连接
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // 在串行端口中打印一条信息
    String currentLine = "";                // 创建一个String来保存来自客户端的传入数据
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // 在客户端连接时进行循环
      currentTime = millis();
      if (client.available()) {             // 如果要从客户端读取字节,
        char c = client.read();             // 然后读取一个字节
        Serial.write(c);                    // 在串行监视器上打印出来
        header += c;
        if (c == '\n') {                    // 如果字节是换行符
          // 如果当前行为空，则一行中有两个换行符。
          // 这是客户端HTTP请求的结束，所以发送一个响应:
          if (currentLine.length() == 0) {
            // HTTP报头总是以响应代码开始(例如HTTP/1.1 200 OK)
            // 然后是content-type，这样客户端就知道接下来会发生什么，然后是空行:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // 打开和关闭gpio
            if (header.indexOf("GET /25/on") >= 0) {
              Serial.println("GPIO 25 on");
              output25State = "on";
              digitalWrite(output25, HIGH);
            } else if (header.indexOf("GET /25/off") >= 0) {
              Serial.println("GPIO 25 off");
              output25State = "off";
              digitalWrite(output25, LOW);
            } else if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
            }
            
            // 显示HTML网页
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS来设置开/关按钮的样式
            // 您可以随意更改背景颜色和字体大小属性以适应您的首选项
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // 网页标题
            client.println("<body><h1>ESP32 Web Server LED </h1>");
            
            // 显示当前状态和GPIO 26的ON/OFF按钮
            client.println("<p>Red LED - State " + output25State + "</p>");
            // 如果output26State为off，则显示ON按钮
            if (output25State=="off") {
              client.println("<p><a href=\"/25/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/25/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // 显示当前状态和GPIO 26的ON/OFF按钮
            client.println("<p> Green LED - State " + output26State + "</p>");
            // 如果output26State为off，则显示ON按钮  
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // HTTP响应以另一个空行结束
            client.println();
            // 跳出while循环
            break;
          } else { // 如果你有一个换行符，那么清除currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // 如果你得到了除了回车符以外的其他字符，
          currentLine += c;      // 将它添加到currentLine的末尾
        }
      }
    }
    // 清除头部变量
    header = "";
    // 关闭连接
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}