<?hh

class A {
  public $a;
  public $b;
  public $c;
  public $d;
  public function __construct($p1, $p2, $p3, $p4) {
    $this->a = $p1;
    $this->b = $p2;
    $this->c = $p3;
    $this->d = $p4;
  }
}
function gen() :AsyncGenerator<mixed,mixed,void>{
  $obj = new A(1, 2, 3, 4);
  foreach ($obj as $key => $val) {
    yield null;
    if($val == 2) {
      $obj->$key = 0;
    } else if($val == 3) {
      var_dump($key);
      unset($obj->$key);
    } else {
      $obj->$key++;
    }
  }
  var_dump($obj);
}

<<__EntryPoint>>
function main() :mixed{
  foreach (gen() as $_) {}
}
