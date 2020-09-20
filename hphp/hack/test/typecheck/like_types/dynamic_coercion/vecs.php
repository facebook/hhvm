<?hh // strict

function f(dynamic $v): vec<dynamic> {
  return $v;
}

function g(dynamic $v): vec<~int> {
  return $v;
}

function h(~vec<int> $v): vec<~int> {
  return $v;
}

function j(~vec<~int> $v): vec<~int> {
  return $v;
}

function bad(vec<~int> $v): ~vec<int> {
  return $v;
}
