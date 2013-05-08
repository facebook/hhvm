<?php

include __DIR__."/builtin_extensions.inc";

class A_DateTime extends DateInterval {
  public $___x;
  public function __clone() {
    $this->___x++;
  }
}
test("DateTime");

function main() {
  echo "================\n";

  $y = new A_DateTime("2012-06-23T11:00:00");
  $y->___y = 73;
  $y2 = clone $y;
  $y2->___y++;
  $y2->modify("+3 days");

  var_dump($y);
  var_dump($y->format('Y-m-d'));
  var_dump($y2);
  var_dump($y2->format('Y-m-d'));
}
main();
