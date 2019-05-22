<?php
namespace foobar;

class foo {
  public function bar(\self $a) { }
}
<<__EntryPoint>> function main() {
$foo = new foo;
$foo->bar($foo); // Ok!
$foo->bar(new stdclass); // Error, ok!
}
