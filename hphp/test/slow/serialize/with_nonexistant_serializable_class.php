<?php
class Foo implements Serializable {
  public function serialize() { return "foobar"; }
  public function unserialize($str) { }
}
$f = new Foo(42);
$fs = serialize($f);
$non_existent_bar = preg_replace('/Foo/', 'Bar', $fs);
var_dump( unserialize($non_existent_bar) );

class Baz {}
$non_serializable_baz = preg_replace('/Foo/', 'Baz', $fs);
var_dump( unserialize($non_serializable_baz) );
