<?php

class P {
  public function foo() {
    bar();
  }
}

function print_backtrace() {
  var_dump(debug_backtrace());
}

trait T {
  public function bar(stdClass $s) {
    print_backtrace();
  }
}
