const Sequelize = require('sequelize');
const env = process.env.NODE_ENV || 'development';
const config = require('../config/config')[env];
const db = {};
const net = require('net'); // socket 통신

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
let port = 8003;
const socket = net.createServer(function (client) {
    console.log('Client connection: ');
    console.log('   local = %s:%s', client.localAddress, client.localPort);
    //console.log('   remote = %s:%s', client.remoteAddress, client.remotePort);
    client.setTimeout(500);
    client.setEncoding('utf8');
    client.on('data', function (data) {
        console.log('Received data from client on port %d: %s', client.remotePort, data);
        let sensor_data = data.split(' ');
        //console.log(sensor_data);

        db.Data.create({
            email: sensor_data[0],
            emg_sensor: sensor_data[1],
            flex_sensor: sensor_data[2]
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

socket.listen(port || 8083, function () {
    console.log('Server listening: ' + JSON.stringify(socket.address())); // 포트 대기중
});

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