<?hh

class thing {
  private $prop;

  function go() :mixed{
    $idx = 'five';
    $instances = dict[];
    $instances[(string)$this->prop] = false;
    return isset($this->prop[$idx]);
  }
}


<<__EntryPoint>>
function main_local_type() :mixed{
$t = new thing;
var_dump($t->go());
}
