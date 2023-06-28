<?hh

class obj {
  public $prop;
  function __wakeup() :mixed{
    $this->prop = 'awake';
  }
}


<<__EntryPoint>>
function main() :mixed{
  $x = 'a:3:{i:0;O:8:"stdClass":1:{i:0;a:1:{i:0;i:1;}}i:1;O:3:"obj":1:{s:4:"prop";R:2;}i:2;R:3;}';
  $y = unserialize($x);

  var_dump($y);
}
