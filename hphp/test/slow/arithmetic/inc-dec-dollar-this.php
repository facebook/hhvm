<?
class Test {
  public function foo() {
    return $this++;
  }
}

$t = new Test();
var_dump($t->foo());
