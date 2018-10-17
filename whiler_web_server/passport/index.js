const local = require('./localStrategy'); // strategy(전략) : 누구를 로그인 시킬 것인가
//const kakao = require('./kakaoStrategy');
const { User } = require('../models');

module.exports = (passport) => {
    passport.serializeUser((user, done) => { // user : 로그인 사용자 정보
        done(null, user.id); 
        // done 함수의 첫 번째 인자는 에러 발생시 사용
        // 두 번째 인자로 user.id 만 저장하라고 명령
    });

    passport.deserializeUser((id, done) => { // 매 요청시 실행, passport.session() 미들웨어가 이 메서드를 호출
        // serializeUser 에서 세션에 저장했던 아이디를 받아 데이터베이스에서 사용자 정보를 조회
        // 조회한 정보를 req.user에 저장
        User.find({ where: { id } })
            .then(user => done(null, user))
            .catch(err => done(err));
    });

    local(passport);
    //kakao(passport);
};