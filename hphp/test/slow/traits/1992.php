<?php

trait MY_TRAIT {
  public function sayHello() {
    echo "Hello World!\n";
  }
}
class MY_CLASS {
  use MY_TRAIT {
    sayHello as falaOi;
  }
}
$a = new MY_CLASS;
$a->falaOi();
?>
