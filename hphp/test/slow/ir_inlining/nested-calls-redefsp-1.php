<?hh

function x() :mixed{ return 3600; }
function a() :mixed{
  return dict['time_mode' => dict['duration' => x()],
               'pipe' => '5m'];
}
function b() :mixed{
  return dict['duration' => x()];
}

function g5m() :mixed{ return '5m'; }

class MyThing {
  public function __construct() {
    $this->duration = 3600;
  }

  function getPipe() :mixed{
    return '5m';
  }

  function getModeQueryData() :mixed{
    return dict['time_mode' => 'history',
                 'pipe' => g5m()];
  }

  function doThings() :mixed{
    for ($i = 0; $i < 10; ++$i) mt_rand();
    $params = dict['duration' => $this->duration];
    return array_merge($this->getModeQueryData(), $params);
  }

  function c() :mixed{
    foreach ($this->doThings() as $k => $v) {
      var_dump($k);
      var_dump($v);
    }
  }
}


<<__EntryPoint>>
function main_nested_calls_redefsp_1() :mixed{
(new MyThing)->c();
}
