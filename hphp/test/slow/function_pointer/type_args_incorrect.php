<?hh

function foo<reify T>(): void {
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

function accepts_string(string $f): void {}

<<__EntryPoint>>
function main(): void {
  accepts_string(foo<int>);
}
