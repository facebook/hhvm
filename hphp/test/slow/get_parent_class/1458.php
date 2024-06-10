<?hh

interface i {
  function test():mixed;
}
class foo implements i {
  function test() :mixed{
    var_dump(get_parent_class());
  }
}
class bar extends foo {
  function test_bar() :mixed{
    var_dump(get_parent_class());
  }
}
class goo extends bar {
  function test_goo() :mixed{
    var_dump(get_parent_class());
  }
}

<<__EntryPoint>>
function main_1458() :mixed{
$bar = new bar;
$foo = new foo;
$goo = new goo;
$foo->test();
$bar->test();
$bar->test_bar();
$goo->test();
$goo->test_bar();
$goo->test_goo();
var_dump(get_parent_class($bar));
var_dump(get_parent_class($foo));
var_dump(get_parent_class($goo));
var_dump(get_parent_class("bar"));
var_dump(get_parent_class("foo"));
var_dump(get_parent_class("goo"));
var_dump(get_parent_class("i"));
var_dump(get_parent_class(""));
var_dump(get_parent_class("[[[["));
var_dump(get_parent_class(" "));
var_dump(get_parent_class(new stdClass));
var_dump(get_parent_class(vec[]));
var_dump(get_parent_class(1));
}
