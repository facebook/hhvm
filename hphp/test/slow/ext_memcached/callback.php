<?php

class Test {
  function __construct() {
    $memc = new Memcached();
    $memc->addServer('localhost', '11211');

    $memc->delete('callback_test');
    var_dump($memc->get('callback_test', array($this, 'ReadThrough')));
    var_dump($memc->get('callback_test'));
  }

  public function ReadThrough($m, $k, &$v) {
    var_dump($m);
    var_dump($k);
    var_dump($v);
    $v = 99;
    return true;
  }
}

new Test();
