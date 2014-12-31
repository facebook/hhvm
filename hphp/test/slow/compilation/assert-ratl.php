<?php

final class X {
  private function foo() {
    $r = array(1 => array());
    $r[1][] = 55;
    $r[1][] = "foo";

    return $r;
  }

  public function bar() {
    $s = $this->foo();

    foreach ($s as $v) {
      if (!$v) {
        continue;
      }

      var_dump($v);
    }
  }
}

(new X)->bar();
