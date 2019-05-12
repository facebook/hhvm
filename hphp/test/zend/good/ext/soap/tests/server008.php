<?php
class Foo {

  function Foo() {
  }

  function test() {
    return $this->str;
  }
}
<<__EntryPoint>> function main() {
$server = new soapserver(null,array('uri'=>"http://testuri.org"));
$server->setclass("Foo");
var_dump($server->getfunctions());
echo "ok\n";
}
