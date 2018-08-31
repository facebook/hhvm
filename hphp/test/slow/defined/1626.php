<?php
class Foo {
  const ZERO        = 0;
  function f() {
    if (defined('self::ZERO')) {
      var_dump(self::ZERO);
    }
    if (defined('THIRTEEN')) {
      var_dump(THIRTEEN);
    }
    if (defined('ONE')) {
      var_dump(ONE);
    }
    $a = 'self::ZERO';
    if (defined($a)) {
      var_dump(self::ZERO);
    }
  }
}
class Bar extends Foo {
  function f() {
    if (defined('self::ZERO')) {
      var_dump(self::ZERO);
    }
    if (defined('parent::ZERO')) {
      var_dump(parent::ZERO);
    }
    $a = 'parent::ZERO';
    if (defined($a)) {
      var_dump(parent::ZERO);
    }
  }
}
class Goo {
  const ZERO = Bar::ZERO;
}


<<__EntryPoint>>
function main_1626() {
define('THIRTEEN', 13);
define('ONE', 1);
$a = 'Foo::ZERO';
if (defined($a)) {
  var_dump(Foo::ZERO);
}
$a = 'Bar::ZERO';
if (defined($a)) {
  var_dump(Bar::ZERO);
}
$a = 'Goo::ZERO';
if (defined($a)) {
  var_dump(Goo::ZERO);
}
$obj = new Foo;
$obj->f();
$obj = new Bar;
$obj->f();
}
