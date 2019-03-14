<?hh

function f<reify T>() {
  var_dump(HH\ReifiedGenerics\getType<T>());
}

f<int>();
