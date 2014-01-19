<?php

namespace {
  function fiz() {
 var_dump(__METHOD__);
 }
  const FIZ = 25;
  const FUZ = 1;
}
namespace foo {
  class bar {
    public function test() {
 echo __CLASS__ . PHP_EOL;
 }
  }
  const FUZ = 2;
  class baz extends bar {
    public function fiz() {
      self::test();
      parent::test();
      static::test();
      bar::test();
    }
  }
  $x = new baz();
  $x->fiz();
  var_dump(true);
  var_dump(false);
  var_dump(null);
  var_dump(INF);
  var_dump(FIZ);
  var_dump(FUZ);
  var_dump(\FUZ);
}
