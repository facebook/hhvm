<?hh

function into_foo(
  foo_typedef $x,
): void {
  FOO_CONST;
  foo_fun();
  new Foo_class();
  Foo_class::foo_static_method();
}

function into_foo_override(
  foo_override_typedef $x,
): void {
  FOO_OVERRIDE_CONST;
  foo_override_fun();
  new Foo_override_class();
  Foo_override_class::foo_override_static_method();
}

function into_bar(
  bar_typedef $x,
): void {
  BAR_CONST;
  bar_fun();
  new Bar_class();
  Bar_class::bar_static_method();
}
