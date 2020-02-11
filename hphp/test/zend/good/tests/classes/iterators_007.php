<?hh
final class Test implements Iterator {
  public $arr = varray[1, 2, 3];
  public $x = 0;

  public function rewind() {
    if ($this->x == 0) throw new Exception(__METHOD__);
    $arr = $this->arr;
    reset(inout $arr);
    $this->arr = $arr;
  }
  public function current() {
    if ($this->x == 1) throw new Exception(__METHOD__);
    $arr = $this->arr;
    $x = current($arr);
    $this->arr = $arr;
    return $x;
  }
  public function key() {
    if ($this->x == 2) throw new Exception(__METHOD__);
    $arr = $this->arr;
    $key = key($arr);
    $this->arr = $arr;
    return $key;
  }
  public function next() {
    if ($this->x == 3) throw new Exception(__METHOD__);
    $arr = $this->arr;
    $n = next(inout $arr);
    $this->arr = $arr;
    return $n;
  }
  public function valid() {
    if ($this->x == 4) throw new Exception(__METHOD__);
    $arr = $this->arr;
    $key = key($arr);
    $this->arr = $arr;
    return $key !== null;
  }
}
<<__EntryPoint>>
function main() {
  $t = new Test();

  while ($t->x < 5) {
    try {
      foreach ($t as $k => $v) {
        echo "Current\n";
      }
    } catch (Exception $e) {
      echo "Caught in ".$e->getMessage()."()\n";
    }
    $t->x++;
  }
  echo "===DONE===\n";
}
