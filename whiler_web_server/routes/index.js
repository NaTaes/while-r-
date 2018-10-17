/* default page */

const express = require('express');
const { isLoggedIn, isNotLoggedIn } = require('./middlewares');

const router = express.Router();

router.get('/profile', isLoggedIn, (req, res) => { // 로그인 한 사람들만 접근
  res.render('profile', { title: '내 정보 - NodeBird', user: req.user });
});

// sign up page //
router.get('/join', isNotLoggedIn, (req, res) => { // 로그인하지 않은사람들만 접근
  res.render('join', {
    title: '회원가입 - NodeBird',
    user: req.user,
    joinError: req.flash('joinError'),
  });
});

router.get('/', (req, res, next) => {
  res.render('main', {
    title: 'NodeBird',
    twits: [],
    user: req.user,
    loginError: req.flash('loginError'),
  });
});
module.exports = router;