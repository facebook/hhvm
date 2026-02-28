<?hh

<<__EntryPoint>>
function test() :mixed{
  if (__hhvm_intrinsics\launder_value(true)) {
    require_once 'parent_by_name_unique_child.1.inc';
  } else {
    require_once 'parent_by_name_unique_child.2.inc';
  }

  require_once 'parent_by_name_unique_child.inc';

  D::printStuff();
}
