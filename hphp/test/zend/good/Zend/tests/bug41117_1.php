<?php
class foo {
  function __construct($this) {
    echo $this."\n";
  }
}
$obj = new foo("Hello world");
?>