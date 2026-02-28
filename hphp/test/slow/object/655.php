<?hh

class g {
  public $v;
  function set($v) :mixed{
    $this->v = $v;
    return $this;
  }
}
function foo() :mixed{
  $z = 1;
  $qd = dict['x' => $z];
  $a = G()->set($qd);
  var_dump($a);
  $qd['e'] = true;
  $b = G()->set($qd);
  var_dump($a);
}
function G() :mixed{
  return new g;
}

<<__EntryPoint>>
function main_655() :mixed{
foo();
}
