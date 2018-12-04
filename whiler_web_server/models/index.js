const Sequelize = require('sequelize');
const env = process.env.NODE_ENV || 'development';
const config = require('../config/config')[env];
const db = {};
const net = require('net'); // socket 통신
const bcrypt = require('bcrypt');

process.setMaxListeners(0);

const sequelize = new Sequelize(
    config.database, config.username, config.password, config,
);

db.sequelize = sequelize;
db.Sequelize = Sequelize;
db.User = require('./user')(sequelize, Sequelize); // 만든 user 테이블 가져오기
db.Post = require('./post')(sequelize, Sequelize); // 만든 post 테이블 가져오기
db.Hashtag = require('./hashtag')(sequelize, Sequelize); // 만든 hashtag 테이블 가져오기
db.Data = require('./data')(sequelize, Sequelize);

/* socket 통신 */
let port = 8003; // sensor data receive
const socket = net.createServer(function (client) {
    console.log('Client connection: ');
    console.log('   local = %s:%s', client.localAddress, client.localPort);
    client.setEncoding('utf8');
    client.on('data', function (data) {
        console.log('Received data from client on port %d: %s', client.remotePort, data);
        console.log('  Bytes received: ' + client.bytesRead);
        let rec_data = data.split(' ');
        let date = new Date().getTime() + (540 * 60 * 1000);

        db.Data.create({ // create 메서드는 db에 데이터 추가
            nick: rec_data[0],
            emg_sensor_up: rec_data[1],
            emg_sensor_down: rec_data[2],
            gyro_sensor: rec_data[3],
            created_at: date
        })
            .then(function (user) {
                // you can now access the newly created task via the variable task
                console.log('success');
            })
            .catch(function (err) {
                // print the error details
                console.log(err, request.body.email);
            });

    });

    client.on('end', function () {
        console.log('Client disconnected');
        // server.getConnections(function (err, count) {
        //     console.log('Remaining Connections: ' + count);
        // });
    });

    client.on('error', function (err) {
        console.log('Socket Error: ', JSON.stringify(err));
    });

    client.on('timeout', function () {
        console.log('Socket Timed out');
    });
});

socket.listen(port || 8083, function () {
    console.log('Server listening: ' + JSON.stringify(socket.address())); // 포트 대기중
});

let port3 = 8005; // UI protocal
const socket3 = net.createServer(function (client) {
    console.log('Client connection: ');
    console.log('   local = %s:%s', client.localAddress, client.localPort);
    client.setEncoding('utf8');
    client.on('data', function (data) {
        console.log('Received data : %s', data);

        let rec_data = data.split(' ');

        if (rec_data[0] == 'login') {
            console.log("protocol login");
            let id = rec_data[1];
            let pw = rec_data[2];

            db.User.findOne({
                attributes: ['password', 'nick'],
                where: {
                    email: id,
                }
            }).then((result) => {
                bcrypt.compare(pw, result.dataValues.password, function (err, res) {
                    if (res == true) {
                        writeData(client, "whiler " + result.dataValues.nick);
                        console.log("send nick : " + result.dataValues.nick);
                    }
                    else {
                        writeData(client, "Error;password");
                        console.log("Error password");
                    }
                });

            }).catch((err) => {
                err = 'Error email';
                console.log(err);
                writeData(client, err);
            });
        }
        else if (rec_data[0] == 'bar') {
            console.log("protocol bar");
            let nick = rec_data[1];
            let day = Number(rec_data[2]);
            let date = new Date();
            date.setDate(date.getDate() - 6 + day);

            let next_date = new Date();
            next_date.setDate(next_date.getDate() - 5 + day);

            db.Data.findAll({
                attributes: ['emg_sensor_up', 'emg_sensor_down', 'gyro_sensor', 'created_at'],
                where: {
                    'nick': nick,
                    'created_at': {
                        $between: [date.toLocaleDateString(), next_date.toLocaleDateString()]
                    }
                },
            }).then((result) => {
                let sum_emg_sensor_up = 0;
                let sum_emg_sensor_down = 0;
                let sum_gyro_sensor = 0;

                for (let i = 0; i < result.length; i++) {
                    //console.log(result[i].dataValues);
                    sum_emg_sensor_up += Number(result[i].dataValues.emg_sensor_up);
                    sum_emg_sensor_down += Number(result[i].dataValues.emg_sensor_down);
                    sum_gyro_sensor += Number(result[i].dataValues.gyro_sensor)
                }

                if (day == 6) {
                    let data = "bar " + sum_emg_sensor_down + " " + sum_emg_sensor_up + " " + sum_gyro_sensor + " 1";
                    console.log(data);
                    writeData(client, data);
                }
                else {
                    let data = "bar " + sum_emg_sensor_down + " " + sum_emg_sensor_up + " " + sum_gyro_sensor + " 0";
                    console.log(data);
                    writeData(client, data);
                }

                // if (result.length == 0) {
                //     writeData(client, "bar 0 0 0 ");
                // }
                // else {
                //     let sum_emg_sensor_up = 0;
                //     let sum_emg_sensor_down = 0;
                //     let sum_gyro_sensor = 0;

                //     for (let i = 0; i < result.length; i++) {
                //         sum_emg_sensor_up += Number(result[i].dataValues.emg_sensor_up);
                //         sum_emg_sensor_down += Number(result[i].dataValues.emg_sensor_down);
                //         sum_gyro_sensor += Number(result[i].dataValues.gyro_sensor)
                //     }
                //     //console.log(i);
                //     if (i != 0) {
                //         let data = "bar " + sum_emg_sensor_down + " " + sum_emg_sensor_up + " " + sum_gyro_sensor + " ";
                //         //console.log(data);
                //         writeData(client, data);
                //     }
                //     else if (i == 0) {
                //         let data = "bar " + sum_emg_sensor_down + " " + sum_emg_sensor_up + " " + sum_gyro_sensor + " ";
                //         //console.log(data);
                //         writeData(client, data);
                //     }
                // }
            })
        }
        else if (rec_data[0] == 'line') {
            console.log("protocol line");
            let nick = rec_data[1];
            let index = rec_data[2];

            db.Data.findAll({
                attributes: ['emg_sensor_up', 'emg_sensor_down', 'gyro_sensor', 'created_at'],
                where: {
                    'nick': nick,
                },
            }).then((result) => {
                let size = result.length;
                let time = JSON.stringify(result[index].dataValues.created_at).split('T');
                time[1] = time[1].slice(0, -2);
                time[0] = time[0].slice(1);
                //console.log(time);

                if (size - 1 == index) {
                    writeData(client, "line " + result[index].dataValues.emg_sensor_down + " " +
                        result[index].dataValues.emg_sensor_up + " " +
                        result[index].dataValues.gyro_sensor + " " +
                        time[0] + " " + time[1] + " " + size + " 1");
                }
                else {
                    writeData(client, "line " + result[index].dataValues.emg_sensor_down + " " +
                        result[index].dataValues.emg_sensor_up + " " +
                        result[index].dataValues.gyro_sensor + " " +
                        time[0] + " " + time[1] + " " + size + " 0");
                }
            });
        }

        else {
            console.log("protocol error");
            writeData(client, "Error protocol");
        }
    });

    client.on('end', function () {
        console.log('Client disconnected');
    });

    client.on('error', function (err) {
        console.log('Socket Error: ', JSON.stringify(err));
    });

    client.on('timeout', function () {
        console.log('Socket Timed out');
    });
});

socket3.listen(port3 || 8083, function () {
    console.log('Server listening: ' + JSON.stringify(socket.address())); // 포트 대기중
});


function writeData(socket, data) {
    var success = !socket.write(data);
    if (!success) {
        (function (socket, data) {
            socket.once('drain', function () {
                console.log('on drain');
                //writeData(socket, data);
            });
        })(socket, data);
    }
}



/* table 관계 설정 부분 */
/* 1:N 관계(user table, post table) */
db.User.hasMany(db.Post);
db.Post.belongsTo(db.User);
// 1:N 관계는 hsaMany 함수는 N 인자를, belongsTo 함수는 1 인자를 받음

/* N:M 관계(hashtag table, post table) */
// N:M 관계는 belongsToMany 함수의 순서는 크게 상관없음
db.Post.belongsToMany(db.Hashtag, { through: 'PostHashtag' });
db.Hashtag.belongsToMany(db.Post, { through: 'PostHashtag' });
// 서로 다른 테이블 관계에서는 칼럼 이름은 postld, hashTagld로 자동 지정됨

/* N:M 관계(user table, user table) */
// 같은 데이블 관계에서는 모델 이름과 컬럼 이름을 따로 정해주어야 함
db.User.belongsToMany(db.User, {
    foreignKey: 'followingId', // foreignKey : 상대 테이블 아이디
    as: 'Followers', // as : 매칭 모델 이름
    through: 'Follow', // through 모델 이름
});

db.User.belongsToMany(db.User, {
    foreignKey: 'followerId',
    as: 'Followings',
    through: 'Follow',
});

/* 게시글 좋아요 및 좋아요 취소 기능 추가 */
db.User.belongsToMany(db.Post, { through: 'Like' });
db.Post.belongsToMany(db.User, { through: 'Like' });

module.exports = db;
