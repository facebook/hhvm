<?hh

class C {
  public function __construct(public int $arg1, public int... $arg2):void { }
  public function test1(mixed $m):void {
    $this->arg1 = HH\FIXME\UNSAFE_CAST<mixed,int>($m);
  }
  public function test2(mixed $m):void {
    $this->arg2 = HH\FIXME\UNSAFE_CAST<mixed,vec<int>>($m);
  }
}
<<__EntryPoint>>
function main():void {
  $c = new C(1, 2, 3, 4);
  var_dump($c->arg1);
  var_dump($c->arg2);
  $c->test1(23);
  var_dump($c->arg1);
  $c->test2(vec[23,45]);
  var_dump($c->arg2);
  try { $c->test1("Broken"); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $c->test2("Broken"); } catch (Exception $e) { var_dump($e->getMessage()); }
}
