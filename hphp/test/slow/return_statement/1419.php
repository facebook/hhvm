<?hh

class q {
}
function g() :mixed{
  return null;
  return new q;
}
function f() :mixed{
  return;
  return new q;
}

<<__EntryPoint>>
function main_1419() :mixed{
var_dump(g());
var_dump(f());
}
