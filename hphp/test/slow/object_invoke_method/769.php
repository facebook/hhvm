<?hh

// with type hints
class C8 {
  public function __invoke(varray $a0) :mixed{
    var_dump($a0);
  }
}

<<__EntryPoint>>
function main_769() :mixed{
$c = new C8;
$c(vec[1, 2, 3]);
}
