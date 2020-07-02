<?hh

function foo<reify T>(): void {
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

function accepts_reified_function_pointer((function(): void) $f): void {
  $f();
}

<<__EntryPoint>>
function main(): void {
  accepts_reified_function_pointer(foo<int>);
}
