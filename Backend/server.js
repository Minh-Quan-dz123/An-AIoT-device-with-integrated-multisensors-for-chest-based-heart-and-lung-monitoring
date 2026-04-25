//1 import các thư viện
import express from "express"; // framework tạo server
import mqtt from "mqtt"; // mqtt <- hiveMQ
import http from "http"; // http 
import { WebSocketServer,  WebSocket } from "ws";// websocker -> frontend: cần có HTTP nền để gắn vào

//2 khởi tạo app
const app = express();
const server = http.createServer(app);

//3 khởi tạo websocket khởi động trên nền http
const wss = new WebSocketServer({ server });

//4 khởi tạo kết nối tới HiveMQ
//4.1 cấu hình ban đầu
const mqttConfig = {
    host: "60294ba1a7534e358c2dc4bc7b7cc9f9.s1.eu.hivemq.cloud",
    port: 8883,
    protocol: "mqtts",          // TLS
    username: "MinhQuan225386",
    password: "12348765@Mq",
    rejectUnauthorized: false   //bỏ qua chứng chỉ SSL tương tự ESP32 hay dùng insecure TLS => giống setInsecure()
};
const mqttClient = mqtt.connect(mqttConfig);

//4.2 khởi tạo topic
// ví dụ const TOPIC = "iot/sensor/data";
// ---------------code của Chiến------------------

//--------------------------------------------------

// ---------------code của Cường------------------

//--------------------------------------------------


// ---------------code của Quân------------------

//--------------------------------------------------


//4.3 connect MQTT
mqttClient.on("connect", () => {
    console.log("Connected to HiveMQ");// in log
    // ----------- subcribe topic--------------------------
});

//4.4 nhận dữ liệu từ MQTT
//Callback này sẽ chạy mỗi khi có 1 message từ bất kỳ topic nào bạn đã subscribe.
mqttClient.on("message", (topic, message) => {
    const data = message.toString();// chuyển sang string
    console.log("MQTT data:", data);

    // 4.4.1. nếu topic = topic của chiến thì xử lý dữ liệu-----------------
    //----------------------------------------------------------------------------

    // 4.4.2. nếu topic = topic của cường thì xử lý dữ liệu-----------------
    //----------------------------------------------------------------------------

    // 4.4.3. nếu topic = topic của quân thì xử lý dữ liệu-----------------
    //----------------------------------------------------------------------------

    // gửi sang tất cả WebSocket clients
    // ví dụ
    wss.clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(data);
        }
    });
});

//5 WebSocket 
//5.1. hàm khi có 1 client kết nối tới
wss.on("connection", (ws) => {
    console.log("Frontend connected");
    ws.send("Connected to WebSocket server");// gửi lại cho client đó
});

// serve frontend
app.use(express.static("public"));

// 5.2 listenta
server.listen(3000, () => {
    console.log("Server running at http://localhost:3000");
});