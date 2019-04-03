<?hh

class C{}
class D{}

function f<reify T>(mixed $x): ?T {
  return $x;
}

var_dump(f<int>(null));
var_dump(f<int>(1));
var_dump(f<C>(null));
var_dump(f<C>(new C()));
var_dump(f<C>(new D()));
