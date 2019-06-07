<?hh
class Test {
  public function foo() {
    return $this++;
  }
}


<<__EntryPoint>>
function main_inc_dec_dollar_this() {
$t = new Test();
var_dump($t->foo());
}
