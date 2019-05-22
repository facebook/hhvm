<?php
class foo {
  function __construct($this) {
    echo $this."\n";
  }
}
<<__EntryPoint>> function main() {
$obj = new foo("Hello world");
}
