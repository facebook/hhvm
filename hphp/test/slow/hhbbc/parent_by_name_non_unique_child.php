<?hh

<<__EntryPoint>>
function test() {
  if (__hhvm_intrinsics\launder_value(true)) {
    require_once 'parent_by_name_non_unique_child.1.inc';
  } else {
    require_once 'parent_by_name_non_unique_child.2.inc';
  }

  require_once 'parent_by_name_non_unique_child.inc';

  D::printStuff();
}
