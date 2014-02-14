<?php
class Foo implements Serializable {
	public function serialize() { return "foobar"; }
	public function unserialize($str) { }
}
$f = new Foo(42);
$fs = serialize($f);
$non_existant_bar = preg_replace('/Foo/', 'Bar', $fs);
var_dump( unserialize($non_existant_bar) );
