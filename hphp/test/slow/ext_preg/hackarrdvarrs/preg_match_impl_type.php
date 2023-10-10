<?hh

function f(): dict {
  $captures = darray[];
  $ret = preg_match_with_matches("%bcd%", "abcdbcdef", inout $captures);
  var_dump($captures);
  var_dump(HH\is_dict($captures));
  return $captures;
}

function g(): dict {
  $captures = darray[];
  $ret = preg_match_all_with_matches("%b[a-z]d%", "abcdbzdef", inout $captures);
  var_dump($captures);
  var_dump(HH\is_dict($captures));
  var_dump(HH\is_dict($captures[0]));
  return $captures;
}

function h(): dict {
  $captures = darray[];
  $ret = preg_match_with_matches(
    '/def$/',
    'abcdef',
    inout $captures,
    PREG_OFFSET_CAPTURE,
    3,
  );
  var_dump($captures);
  var_dump(HH\is_dict($captures));
  var_dump(HH\is_vec($captures[0]));
  return $captures;
}


<<__EntryPoint>>
function main_preg_match_impl_type() :mixed{
f();
g();
h();
}
