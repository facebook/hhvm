<?hh // strict

function f(): darray {
  $captures = [];
  $ret =
    preg_match("%bcd%", "abcdbcdef", &$captures);
  var_dump($captures);
  var_dump(HH\is_darray($captures));
  return $captures;
}

function g(): darray {
  $captures = [];
  $ret =
    preg_match_all("%b[a-z]d%", "abcdbzdef", &$captures);
  var_dump($captures);
  var_dump(HH\is_darray($captures));
  var_dump(HH\is_darray($captures[0]));
  return $captures;
}

function h(): darray {
  $captures = [];
  $ret =
    preg_match('/def$/', 'abcdef', &$captures, PREG_OFFSET_CAPTURE, 3);
  var_dump($captures);
  var_dump(HH\is_darray($captures));
  var_dump(HH\is_varray($captures[0]));
  return $captures;
}

f();
g();
h();
