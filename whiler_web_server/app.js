const express = require("express");
const cookieParser = require("cookie-parser");
const morgan = require("morgan");
const path = require("path");
const session = require("express-session");
const flash = require("connect-flash");
const passport = require("passport"); // passport 미들웨어 장착

require("dotenv").config(); // .env 파일에있는 값을 사용하기위해 선언, process.env.dataname으로 불러올 수 있음
// 세션의 비밀키 값을 하드코딩하면 보안상 문제가 생김
// 이를 해결하기 위해 dotnv 패키지 사용(npm i dotenv 명령어로 설치)

const indexRouter = require("./routes/index");
const authRouter = require('./routes/auth');
const { sequelize } = require("./models"); // models 폴더 연결
// models 폴더 안에있는 index.js 파일을 불러옴, index.js 파일은 require 시 이름을 생략할 수 있음

const passportConfig = require("./passport"); // passport 폴더 연결
// passport 폴더 안의 index.js 파일을 불러옴

const app = express();
sequelize.sync();
passportConfig(passport);

app.set("views", path.join(__dirname, "views"));
app.set("view engine", "pug");
app.set("port", process.env.PORT || 8001); // 사용자가 넣어준 포트가 없을경우 default 값은 8001

app.use(morgan("dev"));
app.use(express.static(path.join(__dirname, "public")));
// express.static은 정적 파일들이 있는 위치를 지정하는 부분
// __dirname은 바로 현재 위치를 가리키는 Node.js의 전역 변수
// 현재 파일 이름을 가리키는 __filename 변수가 있음
// path.join 메소드를 사용하면 디렉토리 주소 구분자가 /인지 \인지 상관없이 환경에 맞게 주소를 완성해줌

app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser(process.env.COOKIE_SECRET));
app.use(session({
    resave: false,
    saveUninitialized: false,
    secret: "nodebirdsecret",
    cookie: {
        httpOnly: true,
        secure: false
    }
}));

app.use(flash()); // 1회성 메세지 표시

app.use(passport.initialize()); // 요청(req 객체)에 passport 설정을 심음, passport 미들웨어 연결
app.use(passport.session()); // req.session 객체에 passport 정보를 저장함, passport 미들웨어 연결
// req.session 객체는 express-session에서 생성하는 것이므로 passport 미들웨어는 express-session 미들웨어보다 뒤에 연결해야함


app.use("/", indexRouter);
app.use("/auth", authRouter);

app.use((req, res, next) => {
    const err = new Error("Not Found");
    err.status = 404;
    next(err);
});

app.use((req, res, next) => {
    res.locals.message = err.message;
    res.locals.error = req.app.get("env") === "development" ? err : {};
    res.status(err.status || 500);
    res.render("error");
});

app.listen(app.get("port"), () => {
    console.log(`${app.get("port")}번 포트에서 대기 중`);
});

/*
var server = net.createServer(function (client) {
    console.log('Client connection: ');
    console.log('   local = %s:%s', client.localAddress, client.localPort);
    //console.log('   remote = %s:%s', client.remoteAddress, client.remotePort);
    client.setTimeout(100);
    client.setEncoding('utf8');
    client.on('data', function (data) {
        console.log('Received data from client on port %d: %s',
            client.remotePort, data.toString());
        console.log('  Bytes received: ' + client.bytesRead);
        //writeData(client, 'Sending: ' + data.toString());
        console.log('  Bytes sent: ' + client.bytesWritten);
    });
    client.on('end', function () {
        console.log('Client disconnected');
        server.getConnections(function (err, count) {
            console.log('Remaining Connections: ' + count);
        });
    });
    client.on('error', function (err) {
        console.log('Socket Error: ', JSON.stringify(err));
    });
    client.on('timeout', function () {
        console.log('Socket Timed out');
    });
});

server.listen(8003, function () {
    console.log('Server listening: ' + JSON.stringify(server.address())); // 서버 대기중
    server.on('close', function () {
        console.log('Server Terminated');
    });
    server.on('error', function (err) {
        console.log('Server Error: ', JSON.stringify(err));
    });
});
*/