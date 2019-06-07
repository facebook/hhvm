<?hh

// with var args
class C9 {
  public function __invoke(...$args) {
    var_dump(count($args));
    var_dump($args);
  }
}

<<__EntryPoint>>
function main_770() {
$c = new C9;
$c();
$c(0);
$c(0, 1);
}
