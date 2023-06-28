<?hh
class X {
  private int $priv = 10;
  public int $val = 1;
  <<__Memoize>>
  public function mul(): int {
    return $this->val * 100;
  }
}


<<__EntryPoint>>
function main_clone_method() :mixed{
$x = new X;
$x->val = 2;
var_dump($x->mul());
$y = clone $x;
$y->val = 3;
var_dump($y->mul());
}
