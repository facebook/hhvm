<?hh

// as a private/protected method
class C6 {
  private function __invoke($a0) :mixed{
    var_dump($a0);
  }
}
class C7 {
  protected function __invoke($a0) :mixed{
    var_dump($a0);
  }
}
 // still works...

<<__EntryPoint>>
function main_768() :mixed{
$c = new C6;
$c(10);
 // still works...
$c = new C7;
$c(20);
}
