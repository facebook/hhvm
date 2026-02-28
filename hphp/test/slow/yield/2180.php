<?hh

trait T {
  abstract protected function gpc():mixed;
  public function gen() :AsyncGenerator<mixed,mixed,void>{
    yield $this->gpc();
  }
}
class C1 {
  use T;
  protected function gpc() :mixed{
    return 1;
  }
}
class C2 {
  use T;
  protected function gpc() :mixed{
    return 2;
  }
}

<<__EntryPoint>>
function main_2180() :mixed{
$obj1 = new C1();
$obj2 = new C2();
$c1 = $obj1->gen();
$c2 = $obj2->gen();
$c1->next();
$c2->next();
var_dump($c1->current());
var_dump($c2->current());
}
