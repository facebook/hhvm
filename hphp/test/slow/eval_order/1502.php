<?hh

class MyIterator implements Iterator{
  private $var = varray[];
  public function __construct($array) {
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
    echo "valid: ",$var?'true':'false',"
";
    return $var;
  }
}

function f() {
 var_dump('f');
 return 0;
 }
function g() {
 var_dump('g');
 return 0;
 }


<<__EntryPoint>>
function main_1502() {
$a = varray[1, 2];
$values = darray['a' => 1, 'b' => 2, 'c' => 3];
$it = new MyIterator($values);
foreach ($it as $a[f()] => $a[g()]) {
  echo $a[0];
  echo "\n";
}
}
