<?hh

function f<reify T>($x) {
  var_dump($x is ?T);
}

f<int>(1);
f<int>(null);
f<int>("hi");


function g<reify T>() {
  $ts = HH\ReifiedGenerics\get_type_structure<T>();
  ksort(&$ts);
  var_dump($ts);
}

function h<reify T>() {
 g<?T>();
 g<@?T>();
}

h<int>();
