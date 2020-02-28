<?hh

// with type hints
class C8 {
  public function __invoke(array $a0) {
    var_dump($a0);
  }
}

<<__EntryPoint>>
function main_769() {
$c = new C8;
$c(varray[1, 2, 3]);
}
