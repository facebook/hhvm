<?php

class C {
  public static function log(Exception $exception) {
    $msg = get_class($exception).': '.$exception->getMessage();
    var_dump($msg);
  }
  public static function setup() {
    set_exception_handler(array(__CLASS__, 'log'));
  }
}
$obj = new C;
$obj->setup();
throw new Exception('test');
