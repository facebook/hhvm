<?hh

function f<reify T>($x) {
  var_dump($x is ?T);
}


function g<reify T>() {
  $ts = HH\ReifiedGenerics\get_type_structure<T>();
  ksort(inout $ts);
  var_dump($ts);
}

function h<reify T>() {
 g<?T>();
 g<@?T>();
}
<<__EntryPoint>>
function main_entry(): void {

  f<int>(1);
  f<int>(null);
  f<int>("hi");

  h<int>();
}
