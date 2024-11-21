<?hh

enum Opaque: int {
  A = 1;
}
enum AsInt: int as int {
  A = 1;
}

function f(enum<Opaque> $o): HH\enumname<Opaque> {
  return $o;
}
function g(enum<Opaque> $o): enum<int> {
  return $o;
}
function h(HH\enumname<Opaque> $o): HH\enumname<int> {
  return $o;
}
function j(enum<AsInt> $i): enum<int> {
  return $i;
}
function k(HH\enumname<AsInt> $i): HH\enumname<int> {
  return $i;
}
