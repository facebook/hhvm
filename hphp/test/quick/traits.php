<?hh
const FOO = 123;
trait T {
  private $blah = FOO;
  public function test() :mixed{
    var_dump($this->blah);
  }
}
class C {
  use T;
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->test();
}
