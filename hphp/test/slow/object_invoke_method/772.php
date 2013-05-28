<?php

class X {
}
class Y {
  public function __invoke() {
}
}
var_dump(is_callable(new X));
var_dump(is_callable(new Y));
