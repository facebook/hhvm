<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public $prop1;
  public $prop2;
  private $prop3;
  private $prop4;
}


<<__EntryPoint>>
function main_reflection() :mixed{
$reflect = new ReflectionClass('Foo');
var_dump($reflect->getProperties());
}
