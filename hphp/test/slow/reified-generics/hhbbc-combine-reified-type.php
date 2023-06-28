<?hh

function f<reify T>($x) :mixed{
  var_dump($x is ?T);
}


function g<reify T>() :mixed{
  $ts = HH\ReifiedGenerics\get_type_structure<T>();
  ksort(inout $ts);
  var_dump($ts);
}

function h<reify T>() :mixed{
 g<?T>();
 g<<<__Soft>> ?T>();
}
<<__EntryPoint>>
function main_entry(): void {

  f<int>(1);
  f<int>(null);
  f<int>("hi");

  h<int>();
}
