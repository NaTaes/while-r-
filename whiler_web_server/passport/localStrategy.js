const LocalStrategy = require('passport-local').Strategy;
const bcrypt = require('bcrypt');

const { User } = require('../models');

module.exports = (passport) => {
    passport.use(new LocalStrategy({
        // urlencoded 미들웨어가 해석한 req.body의 값들을 usernameField, passwordField 에 연결합니다.
        usernameField: 'email', // req.body.email
        passwordField: 'password', // req.body.password
    }, async (email, password, done) => { // done(서버에러), done(null, 사용자 정보)
        try {
            const exUser = await User.find({ where: { email } }); // email 검사
            if (exUser) {//비밀 번호 검사
                const result = await bcrypt.compare(password, exUser.password);
                // bcrypt 비밀번호를 암호화 하는 미들웨어

                if (result) { // 비밀번호 맞음, 로그인 성공
                    done(null, exUser);
                }
                else {
                    done(null, false, { message: '비밀번호가 일치하지 않습니다.' });
                }
            }
            else {
                done(null, false, { message: '가입되지 않은 회원입니다.' }); // done(null, false, 실패 정보)
            }
        }
        catch (error) {
            console.error(error);
            done(error);
        }
    }));
};