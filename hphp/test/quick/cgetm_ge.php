<?hh

class ary implements ArrayAccess {
  private $c;
  public function __construct($c = 2) {
    $this->c = $c;
  }
  public function offsetExists($i) {
    return true;
  }
  public function offsetGet($i) {
    return $i * $this->c;
  }
  public function offsetSet($i, $v) {
  }
  public function offsetUnset($i) {
  }
}

<<__EntryPoint>>
function cgetm_ge() {
  $GLOBALS['gArray'] = array(1, 2, 'bob', 'cat');
  $GLOBALS['gObj'] = new ary(4);
  $GLOBALS['gStr'] = '01234567890';


  var_dump($GLOBALS['gArray'][2]);
  var_dump($GLOBALS['gObj'][6]);
  var_dump($GLOBALS['gStr'][3]);

  $idx = array(2, 6, 3);
  foreach (array('gArray', 'gObj', 'gStr') as $dyn) {
    var_dump($GLOBALS[$dyn][array_shift(&$idx)]);
  }

  try {
    $a = $GLOBALS['doesnt_exist'][12];
    var_dump($a);
  } catch (Exception $e) { echo $e->getMessage()."\n"; }

  foreach (HH\global_keys() as $k) {
    if ($k == 'doesnt_exist') {
      echo "has key $k\n";
    }
  }
}
