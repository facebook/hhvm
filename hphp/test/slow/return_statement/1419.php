<?hh

class q {
}
function g() {
  return null;
  return new q;
}
function f() {
  return;
  return new q;
}

<<__EntryPoint>>
function main_1419() {
var_dump(g());
var_dump(f());
}
