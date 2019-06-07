<?hh

class ary implements ArrayAccess {
  public function __construct() {
    echo "Constructing\n";
  }
  public function offsetExists($i) {
    return true;
  }
  public function offsetGet($i) {
    return $i . $i;
  }
  public function offsetSet($i, $v) {
  }
  public function offsetUnset($i) {
  }
}

<<__EntryPoint>> function main() {
  $a = array(null, new ary(), array('cat' => 'meow', 'dog' => 'woof'));
  var_dump($a[0]['unused']);
  var_dump($a[1]['tick']);
  var_dump($a[2]['dog']);
  try { var_dump($a[3]['unused']); } catch (Exception $e) { echo $e->getMessage()."\n"; }

  apc_store('widget', $a);
  unset($a);
  $a = apc_fetch('widget');
  var_dump($a[0]['unused']);
  var_dump($a[1]['tock']);
  var_dump($a[2]['cat']);
  try { var_dump($a[3]['unused']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
