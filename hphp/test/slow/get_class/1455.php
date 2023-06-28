<?hh

class foo {
  function bar () :mixed{
    var_dump(get_class());
    var_dump(get_class(null));
  }
}
class foo2 extends foo {
}

<<__EntryPoint>>
function main_1455() :mixed{
$f1 = new foo;
$f2 = new foo2;
var_dump(get_class($f1));
$f1->bar();
$f2->bar();
var_dump(get_class("qwerty"));
var_dump(get_class($f1));
var_dump(get_class($f2));
}
