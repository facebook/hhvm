<?php

class MyClass {
    public function __invoke() {
      var_dump('called');
      return $this;
    }
}

(new MyClass())()();
