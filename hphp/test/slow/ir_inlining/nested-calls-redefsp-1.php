<?hh

function x() { return 3600; }
function a() {
  return darray['time_mode' => darray['duration' => x()],
               'pipe' => '5m'];
}
function b() {
  return darray['duration' => x()];
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
    return darray['time_mode' => 'history',
                 'pipe' => g5m()];
  }

  function doThings() {
    for ($i = 0; $i < 10; ++$i) mt_rand();
    $params = darray['duration' => $this->duration];
    return $this->getModeQueryData() + $params;
  }

  function c() {
    foreach ($this->doThings() as $k => $v) {
      var_dump($k);
      var_dump($v);
    }
  }
}


<<__EntryPoint>>
function main_nested_calls_redefsp_1() {
(new MyThing)->c();
}
