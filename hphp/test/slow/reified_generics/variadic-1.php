<?hh

function test<reify T>(
  string ...$xs
): void {
  var_dump($xs);
  var_dump(HH\ReifiedGenerics\getType<T>());
}

<<__EntryPoint>>
function main(): void {
  test<int>('foo', 'bar');
}
