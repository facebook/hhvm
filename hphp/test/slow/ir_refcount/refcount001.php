<?hh

class X {
  private $rc_prop;
  function __construct(varray $x) {
    $this->rc_prop = $x;
  }
  function thing() {
    return $this->rc_prop;
  }
}

function go() {
  var_dump((new X(varray[new stdClass]))->thing());
  var_dump((new X(varray[new stdClass]))->thing());
  var_dump((new X(varray[new stdClass]))->thing());
  var_dump((new X(varray[new stdClass]))->thing());
  var_dump((new X("yoyoyo"))->thing());
}


<<__EntryPoint>>
function main_refcount001() {
go();
}
