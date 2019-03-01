<?hh

function g<reify T1, reify T2>() {
  var_dump(HH\ReifiedGenerics\getType<T1>());
  var_dump(HH\ReifiedGenerics\getType<T2>());
}

function f<reify T1, reify T2>($x, $y) {
  g<(int, (T1, T2)), T1>();
}

f<(int, int), int>(1, 2);
