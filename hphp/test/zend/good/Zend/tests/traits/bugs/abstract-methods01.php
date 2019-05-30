<?php

trait THello {
  public abstract function hello();
}

class TraitsTest {
    use THello;
}

<<__EntryPoint>> function main() {
error_reporting(E_ALL);
$test = new TraitsTest();
$test->hello();
}
