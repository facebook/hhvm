<?php

print "Test begin\n";

interface I {
  public function a(array $a = null);
}
class C implements I {
  public function a(array $a) {}
}

print "Test end\n";
