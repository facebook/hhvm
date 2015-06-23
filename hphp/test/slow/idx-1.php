<?hh

// Test classes
class MyArray implements ArrayAccess {
  private $arr = array('a' => 'apple', 'b' => null);
  public function offsetExists($index) {
    return array_key_exists($index, $this->arr);
  }
  public function offsetGet($index) {
    return $this->arr[$index];
  }
  public function offsetSet($index, $newvalue) {
    $this->arr[$index] = $newvalue;
  }
  public function offsetUnset($index) {
    unset($this->arr[$index]);
  }
}

class MyIndex extends MyArray implements ConstIndexAccess {
  public function at($index) {
    return $this->arr[$index];
  }
  public function get($index) {
    return $this->arr[$index];
  }
  public function containsKey($index) {
    return true;
  }
}

function main() {
  $o = new stdClass();
  $a = new MyArray();
  $op = new MyIndex();
  $op['prop_name'] = 43;
  $v = Vector { 'a' , 'b' };
  $m = Map { 'a' => 2, 'b' => 'c' };
  $s = 'hello';

  // Arrays
  var_dump(idx(array(2 => 'h', 3 => 'i', 4 => 'j'), 4, null));
  var_dump(idx(array('hello' => 42), 'hello', 31337));
  var_dump(idx(array(2 => false), 2, true));
  var_dump(idx(array('world' => 1), 'hello', $o));
  var_dump(idx(array(2 => null), 2, 'not_reached'));
  var_dump(idx(array(), 2, 'not_reached'));
  var_dump(idx(array(), 'absent'));
  var_dump(idx(array(), null, 5));
  echo "\n";

  // Collections
  var_dump(idx($m, 'a', 4));
  var_dump(idx($m, 'c', 'f'));
  var_dump(idx($m, 'absent'));
  var_dump(idx($m, null, 'f'));
  var_dump(idx($v, 0, 'd'));
  var_dump(idx($v, 2, 'd'));
  var_dump(idx($v, 31337));
  var_dump(idx($v, null, 'd'));
  echo "\n";

  // strings
  var_dump(idx($s, 1, 'abc'));
  var_dump(idx($s, 5, 'abc'));
  var_dump(idx($s, '1'));
  var_dump(idx($s, '5'));
  var_dump(idx($s, 'wtf'));
  var_dump(idx($s, 8));
  var_dump(idx($s, null, 'abc'));
  echo "\n";

  // ArrayAccess
  var_dump(idx($a, 'a', null));
  var_dump(idx($a, 'b', 'orange'));
  var_dump(idx($a, 'c', 'orange'));
  var_dump(idx($a, 'absent'));
  var_dump(idx($a, null, 'orange'));
  echo "\n";

  // ConstIndexAccess
  var_dump(idx($op, 'not_real', 43)); // absent property despite containsKey
  var_dump(idx($op, 'prop_name', 43));
  var_dump(idx($op, 'absent')); // absent property despite containsKey
  var_dump(idx($op, null, 43));
  echo "\n";

  // null (because PHP)
  var_dump(idx(null, 'absent'));
  var_dump(idx(null, 'not_reached', 'wtf'));
  echo "\n";

  // too few arguments
  var_dump(idx($a));
  var_dump(idx());
}

main();
