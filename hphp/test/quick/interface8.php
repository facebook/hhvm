<?php

print "Test begin\n";

interface A {
  public function a(array $a = null);
}
class B implements A {
  public function a($a) {}
}

print "Test end\n";
