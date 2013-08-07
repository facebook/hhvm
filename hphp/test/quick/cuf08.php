<?php

class Test {

  public function __call($method, $args) {
    var_dump($args);
  }

  public static function __callStatic($method, $args) {
    var_dump($args);
  }

  public function normal($args) {
    var_dump($args);
  }

}

$test = new Test();
call_user_func_array(array($test, 'magic'), array('bur' => 'bar'));
call_user_func_array(array($test, 'normal'), array('badum' => 'tss'));
call_user_func_array('Test::hi', array('bleep', 'bloop'));
$test->hi('hello world!');
