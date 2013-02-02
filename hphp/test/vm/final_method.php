<?php

print "Test begin\n";

class C {
  final public function f() {}
}
class D extends C {
  public function f() {}
}

print "Test end\n";
