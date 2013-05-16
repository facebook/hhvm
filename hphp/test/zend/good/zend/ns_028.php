<?php
require "ns_028.inc";

class Foo {
  function __construct() {
  	echo "Method - ".__CLASS__."::".__FUNCTION__."\n";
  }
  static function Bar() {
  	echo "Method - ".__CLASS__."::".__FUNCTION__."\n";
  }
}

$x = new Foo;
Foo\Bar();
$x = new Foo\Foo;
Foo\Foo::Bar();
\Foo\Bar();