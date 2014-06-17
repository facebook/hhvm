<?php

function x() { return 3600; }
function a() {
  return array('time_mode' => array('duration' => x()),
               'pipe' => '5m');
}
function b() {
  return array('duration' => x());
}

function g5m() { return '5m'; }

class MyThing {
  public function __construct() {
    $this->duration = 3600;
  }

  function getPipe() {
    return '5m';
  }

  function getModeQueryData() {
    return array('time_mode' => 'history',
                 'pipe' => g5m());
  }

  function doThings() {
    for ($i = 0; $i < 10; ++$i) mt_rand();
    $params = array('duration' => $this->duration);
    return $this->getModeQueryData() + $params;
  }

  function c() {
    foreach ($this->doThings() as $k => $v) {
      var_dump($k);
      var_dump($v);
    }
  }
}

(new MyThing)->c();
