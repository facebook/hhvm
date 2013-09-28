<?php

class A implements Countable {
 public function count() {
 return 1;
}
}
 $obj = new A();
 var_dump(count($obj));
