<?hh

class C implements ArrayAccess {
  public $arr = array();
  public function offsetExists($k) {
    echo "offsetExists called\n";
    return isset($this->arr[$k]);
  }
  public function offsetGet($k) {
    echo "offsetGet called\n";
    return $this->arr[$k];
  }
  public function offsetSet($k, $v) {
    echo "offsetSet called\n";
    if (is_null($k)) {
      $this->arr[] = $v;
    } else {
      $this->arr[$k] = $v;
    }
  }
  public function offsetUnset($k) {
    echo "offsetUnset called\n";
    unset($this->arr[$k]);
  }
}

function test($o, $f) {
  echo "==== class " . get_class($o) . " ====\n";

  $f($o);
  array_key_exists(0, $o);
  isset($o[0]);
  empty($o[0]);
  var_dump($o[0]);
  try {
    unset($o[0]);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}

<<__EntryPoint>>
function main() {
  test(new C(),  $o ==> $o[0] = 1);
  test(Vector{}, $o ==> $o[] = 1);
  test(Map{},    $o ==> $o[0] = 1);
  test(Set{},    $o ==> $o[] = 0);
}
