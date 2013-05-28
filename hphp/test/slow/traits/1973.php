<?php

trait A {
  public function smallTalk() {
    echo "a\n";
  }
  public function bigTalk($n) {
    if ($n == 0) return;
    echo "A$n\n";
    $m = $n - 1;
    $this->bigTalk($m);
  }
}
trait B {
  public function smallTalk() {
    echo "b\n";
  }
  public function bigTalk($n) {
    if ($n == 0) return;
    echo "B$n\n";
    $this->bigTalk($n - 1);
  }
}
class Talker {
  use A, B {
    B::smallTalk insteadof A;
    A::bigTalk insteadof B;
    B::bigTalk as bTalk;
  }
}
$talker = new Talker();
$talker->smallTalk();
$talker->bigTalk(1);
$talker->bTalk(2);
?>
