<?hh

function f<reify T>() {
  var_dump(HH\ReifiedGenerics\getTypeStructure<T>());
}
<<__EntryPoint>> function main(): void {
f<int>();
}
