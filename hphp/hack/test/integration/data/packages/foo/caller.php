<?hh

function foo_caller3(foo_typedef $x): void {
  FOO_CONST;
}

function foo_caller1(): void {
  foo_fun();
}

function foo_caller2(): void {
  new Foo_class();
}

function foo_caller4(): void {
  Foo_class::foo_static_method();
}

function foo_override_used_caller(
  foo_override_used $x
): void {}
