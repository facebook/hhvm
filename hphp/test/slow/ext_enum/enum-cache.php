<?hh

if (apc_add('i_was_already_run', 1)) {
  class Foo { const ABC = 42; }
} else {
  class Foo { const ABC = 24; }
}

enum Bar: int {
  VALUE = Foo::ABC;
}

var_dump(Bar::getValues());
