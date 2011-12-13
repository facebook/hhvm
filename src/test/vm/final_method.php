<?php

// XXX We currently have disabled enforcement of the 'final' keyword.
// When we re-enable it we will need to update the .exp file for this
// test accordingly.

print "Test begin\n";

class C {
  final public function f() {}
}
class D extends C {
  public function f() {}
}

print "Test end\n";
