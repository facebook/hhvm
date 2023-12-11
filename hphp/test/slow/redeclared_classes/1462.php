<?hh

class base1 {
}
class base2 {
}

<<__EntryPoint>>
function main_1462() :mixed{
  if (__hhvm_intrinsics\launder_value(true)) {
    include '1462-1.inc';
  } else {
    include '1462-2.inc';
  }
  $foo = "foo";
  $y = new a;
  var_dump(a::foo());
  var_dump(a::$foo());
  var_dump(call_user_func(vec['a','foo']));
  var_dump(a::$astat);
  var_dump(a::$a1stat);
  var_dump(a::aconst);
  var_dump(a::a1const);
  var_dump(method_exists('a',"foo"));
  var_dump(method_exists($y,"foo"));
  var_dump(property_exists("a","astat"));
  var_dump(property_exists("a","a1stat"));
  var_dump(property_exists("a","a2stat"));
  var_dump(get_parent_class($y));
  var_dump(is_subclass_of("a", "base1"));
  var_dump(is_subclass_of("a", "base2"));
  var_dump(get_object_vars($y));
}
