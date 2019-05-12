<?hh

function f<reify T>() {
  var_dump(HH\ReifiedGenerics\getType<T>());
}
<<__EntryPoint>> function main(): void {
f<int>();
}
