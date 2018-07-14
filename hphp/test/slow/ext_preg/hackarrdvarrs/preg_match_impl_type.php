<?hh // strict

function f(): dict {
  $captures = [];
  $ret =
    preg_match("%bcd%", "abcdbcdef", &$captures);
  var_dump($captures);
  var_dump(HH\is_dict($captures));
  return $captures;
}

function g(): dict {
  $captures = [];
  $ret =
    preg_match_all("%b[a-z]d%", "abcdbzdef", &$captures);
  var_dump($captures);
  var_dump(HH\is_dict($captures));
  var_dump(HH\is_dict($captures[0]));
  return $captures;
}

function h(): dict {
  $captures = [];
  $ret =
    preg_match('/def$/', 'abcdef', &$captures, PREG_OFFSET_CAPTURE, 3);
  var_dump($captures);
  var_dump(HH\is_dict($captures));
  var_dump(HH\is_vec($captures[0]));
  return $captures;
}

f();
g();
h();
