const express = require('express');
const passport = require('passport');
const bcrypt = require('bcrypt');
const { isLoggedIn, isNotLoggedIn } = require('./middlewares');
const { User } = require('../models');

const router = express.Router();

router.post('/join', isNotLoggedIn, async (req, res, next) => {
  const { email, nick, password } = req.body;
  try {
    const exUser = await User.find({ where: { email } });
    if (exUser) { // 아이디가 이미 가입되있다면
      req.flash('joinError', '이미 가입된 이메일입니다.'); // 1회성 메세지로 보여줌
      return res.redirect('/join'); // 회원가입 페이지로 다시 돌려보냄
    }
    const hash = await bcrypt.hash(password, 12); // bcrypt로 password를 암호화 함, 뒤에 숫자가 암화화량을 늘려줌(높을수록 오래걸림)
    await User.create({
      email,
      nick,
      password: hash,
    });
    return res.redirect('/'); // 회원가입후 메인페이지로 이동 시킴
  }
  catch (error) {
    console.error(error);
    return next(error);
  }
});

// POST /auth/login
router.post('/login', isNotLoggedIn, (req, res, next) => { // req.body.email, req.body.password
  passport.authenticate('local', (authError, user, info) => { // localStrategy.js 의 성공 실패가 authError, user, info 로 전달
    if (authError) { // 에러인 경우
      console.error(authError);
      return next(authError);
    }
    if (!user) { // 사용자 정보가 없는경우
      req.flash('loginError', info.message);
      return res.redirect('/');
    }
    return req.login(user, (loginError) => { // 성공인 경우, req.user 세션에 저장
      if (loginError) {
        console.error(loginError);
        return next(loginError);
      }
      return res.redirect('/');
    });
  })(req, res, next); // 미들웨어 내의 미들웨어에는 (req, res, next)를 붙입니다.
});

// GET /auth/logout
router.get('/logout', isLoggedIn, (req, res) => {
  req.logout();
  req.session.destroy(); // 세션 삭제
  res.redirect('/');
});

router.get('/kakao', passport.authenticate('kakao'));

router.get('/kakao/callback', passport.authenticate('kakao', {
  failureRedirect: '/',
}), (req, res) => {
  res.redirect('/');
});

module.exports = router;