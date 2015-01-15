<?php

class X {
  private $someArrayMember = array();
  private $someOtherMember;

  function __construct(array $x) { $this->someArrayMember = $x; }

  final function myfunc() {
    /*
     * At the time of this test's creation, this sequence of code generated
     * redundant loads from the property for the CGetM and the SetM.  These two
     * temps may alias, so before decrefing to overwrite it with an array(), it
     * needs to observe the may-alias sets to ensure the incref for the return
     * value isn't pushed past that decref.  It will probably stop testing that
     * if we start running weaken_decrefs before load elimination.
     */
    $thing = $this->someArrayMember;
    $this->someArrayMember = array();
    $this->someOtherMember = null;
    return $thing;
  }
}

function go(X $x) {
  var_dump($x->myfunc());
}

for ($i = 0; $i < 30; ++$i) {
  go(new X(array(1,2,new stdclass)));
}
