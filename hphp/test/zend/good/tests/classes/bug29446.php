<?php

class testClass {
  const TEST_CONST = 'test';
  const TEST_CONST = 'test1';

  function testClass() {
    echo self::TEST_CONST;
  }
}
<<__EntryPoint>> function main() {
$test = new testClass;
}
