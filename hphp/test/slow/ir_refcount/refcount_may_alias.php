<?hh

class X {
  private $someArrayMember = vec[];
  private $someOtherMember;

  function __construct(varray $x) { $this->someArrayMember = $x; }

  final function myfunc() :mixed{
    /*
     * At the time of this test's creation, this sequence of code generated
     * redundant loads from the property for the CGetM and the SetM.  These two
     * temps may alias, so before decrefing to overwrite it with an vec[], it
     * needs to observe the may-alias sets to ensure the incref for the return
     * value isn't pushed past that decref.  It will probably stop testing that
     * if we start running weaken_decrefs before load elimination.
     */
    $thing = $this->someArrayMember;
    $this->someArrayMember = vec[];
    $this->someOtherMember = null;
    return $thing;
  }
}

function go(X $x) :mixed{
  var_dump($x->myfunc());
}


<<__EntryPoint>>
function main_refcount_may_alias() :mixed{
for ($i = 0; $i < 30; ++$i) {
  go(new X(vec[1,2,new stdClass]));
}
}
