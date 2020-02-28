<?hh

class MyIterator implements Iterator{
  private $var = varray[];
  public function __construct($array)  {
    echo "constructing
";
    if (is_array($array)) {
        $this->var = $array;
    }
  }
  public function rewind() {
    echo "rewinding
";
    $__var = $this->var;
    reset(inout $__var);
    $this->var = $__var;
  }
  public function current() {
    $__var = $this->var;
    $var = current($__var);
    $this->var = $__var;
    echo "current: $var
";
    return $var;
  }
  public function key() {
    $__var = $this->var;
    $var = key($__var);
    $this->var = $__var;
    echo "key: $var
";
    return $var;
  }
  public function next() {
    $__var = $this->var;
    $var = next(inout $__var);
    $this->var = $__var;
    echo "next: $var
";
    return $var;
  }
  public function valid() {
    $var = $this->current() !== false;
    echo "valid: $var
";
    return $var;
  }
}
function getIter() {
  $values = varray[1,2,3];
  $it = new MyIterator($values);
  return $it;
}
class MyIteratorAggregate implements IteratorAggregate {
  public function getIterator() {
    return getIter();
  }
}

<<__EntryPoint>>
function main_439() {
$values = varray[1,2,3];
$it = new MyIterator($values);
foreach ($it as $a => $b) {
  print "$a: $b
";
}
foreach (getIter() as $a => $b) {
  print "$a: $b
";
}
$obj = new MyIteratorAggregate();
foreach ($obj as $a => $b) {
  print "$a: $b
";
}
}
