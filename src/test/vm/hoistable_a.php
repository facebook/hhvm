<?php

print "Test begin\n";

if (!isset($g)) {
  class A {}
}

class B extends A {}

print "Test end\n";
