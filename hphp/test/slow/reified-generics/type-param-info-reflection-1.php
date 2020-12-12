<?hh

function f() {}
function g<reify T1, T2, <<__Warn, __Soft>> reify T3>() {}

class C {
  public function h<reify T1, T2, <<__Warn, __Soft>> reify T3>() {}
}

<<__EntryPoint>>
function main() {
  var_dump((new ReflectionFunction('f'))->getReifiedTypeParamInfo());
  var_dump((new ReflectionFunction('g'))->getReifiedTypeParamInfo());
  var_dump((new ReflectionMethod('C', 'h'))->getReifiedTypeParamInfo());
}
