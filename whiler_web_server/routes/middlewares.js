exports.isLoggedIn = (req, res, next) => {
  if (req.isAuthenticated()) { // 로그인 여부
    next();
  } else {
    res.status(403).send('로그인 필요');
  }
};

exports.isNotLoggedIn = (req, res, next) => {
  if (!req.isAuthenticated()) { // 로그인 안했을 시
    next();
  } else {
    res.redirect('/');
  }
};