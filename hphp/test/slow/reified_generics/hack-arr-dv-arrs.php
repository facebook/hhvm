<?hh

function f<reify T>() {
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}
<<__EntryPoint>> function main(): void {
f<int>();
}
