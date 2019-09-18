<?hh

enum Bar: int {
  VALUE = Foo::ABC;
}
<<__EntryPoint>>
function main_entry(): void {

  if (apc_add('i_was_already_run', 1)) {
    include 'enum-cache-1.inc';
  } else {
    include 'enum-cache-2.inc';
  }

  var_dump(Bar::getValues());
}
