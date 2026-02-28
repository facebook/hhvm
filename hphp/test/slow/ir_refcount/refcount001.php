<?hh

class X {
  private $rc_prop;
  function __construct(varray $x) {
    $this->rc_prop = $x;
  }
  function thing() :mixed{
    return $this->rc_prop;
  }
}

function go() :mixed{
  var_dump((new X(vec[new stdClass]))->thing());
  var_dump((new X(vec[new stdClass]))->thing());
  var_dump((new X(vec[new stdClass]))->thing());
  var_dump((new X(vec[new stdClass]))->thing());
  var_dump((new X("yoyoyo"))->thing());
}


<<__EntryPoint>>
function main_refcount001() :mixed{
go();
}
