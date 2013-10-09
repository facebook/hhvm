<?php

class testClass
{
  protected $property = 'value';

  public function testInclude()
  {
    include __DIR__ . '/class_include_nested_1.inc';
  }
}

$test = new testClass();
$test->testInclude();
