<?hh // strict

enum Foo: int as int {
  FOO = 1;
}

function foo<T>(): enumname<T> {
  return Foo::class;
}

function bar<T>(enumname<T> $input): void {}

type enumname<T> = HH\enumname<T>;

<<__EntryPoint>>
function main_enumname(): void {
  bar(Foo::class);
  var_dump(foo());
}
