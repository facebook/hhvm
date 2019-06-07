<?hh

trait MY_TRAIT {
  public function sayHello() {
    echo "Hello World!\n";
  }
}
class MY_CLASS {
  use MY_TRAIT {
    sayHello as falaOi;
  }
}
<<__EntryPoint>> function main() {
$a = new MY_CLASS;
$a->falaOi();
}
