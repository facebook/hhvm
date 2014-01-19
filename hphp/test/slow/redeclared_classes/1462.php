<?php

class base1 {
}
class base2 {
}
if (true) {
  class a extends base1 {
    const aconst = "firstA";
    const a1const = 0;
    static $astat = 1;
    static $a1stat = 1;
    function __construct() {
 echo "first def made
";
 }
    static function foo() {
 return 1;
}
  }
}
 else {
  class a extends base2 {
    const aconst = "secondA";
    const a2const = 0;
    static $astat = 2;
    static $a2stat = 2;
    function __construct() {
 echo "second def made
";
 }
    static function foo() {
 return 2;
}
  }
}
$foo = "foo";
$y = new a;
var_dump(a::foo());
var_dump(a::$foo());
var_dump(call_user_func(array('a','foo')));
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
