<?php
if (1) {
  class Foo {
    const TABLE = "foo";
    public $table = self::TABLE;
  }
}
if (1) {
  class Bar extends Foo {
    const TABLE = "bar";
  }
}
$bar = new Bar();
var_dump($bar->table);
?>