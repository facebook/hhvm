<?hh

// with var args
class C9 {
  public function __invoke(...$args) :mixed{
    var_dump(count($args));
    var_dump($args);
  }
}

<<__EntryPoint>>
function main_770() :mixed{
$c = new C9;
$c();
$c(0);
$c(0, 1);
}
