<?hh

function f() :mixed{}
function g<reify T1, T2, <<__Warn, __Soft>> reify T3>() :mixed{}

class C {
  public function h<reify T1, T2, <<__Warn, __Soft>> reify T3>() :mixed{}
}

<<__EntryPoint>>
function main() :mixed{
  var_dump((new ReflectionFunction('f'))->getReifiedTypeParamInfo());
  var_dump((new ReflectionFunction('g'))->getReifiedTypeParamInfo());
  var_dump((new ReflectionMethod('C', 'h'))->getReifiedTypeParamInfo());
}
