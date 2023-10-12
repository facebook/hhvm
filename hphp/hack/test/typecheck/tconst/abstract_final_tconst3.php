<?hh // strict

abstract class Foo {
  const type T = int;
}

abstract final class Bar {
  const type T = string;
}

function test(Foo::T $x, Bar::T $y): void {}

function run(): void {
  test(0, '0');
}
