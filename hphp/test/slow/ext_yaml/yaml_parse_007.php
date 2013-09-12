<?php
class A {
  public $one = 1;

  public function show_one() {
    echo $this->one;
  }
}

var_dump(yaml_parse('
---
a: !php/object "O:1:\"A\":1:{s:3:\"one\";i:1;}"
...
'));
?>
