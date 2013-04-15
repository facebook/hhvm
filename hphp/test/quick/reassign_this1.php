<?php
class Foo {
  public $x;
}
$this =& new Foo();
echo $this->x;
