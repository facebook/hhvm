<?hh // strict

enum Foo: int as int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

function foo<T>(HH\enumname<T> $input): void {}

function bar<T>(enumname<T> $input): void {}

type enumname<T> = HH\enumname<T>;
const enumname<arraykey> BUILTIN_ENUM = HH\BUILTIN_ENUM;

function test(): void {
  foo(Foo::class);
  bar(Foo::class);
}
