<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  const C = "C::C";
  private $np = C::C;
  protected $nq = C::C;
  public $nr = C::C;
  private $p = 1;
  protected $q = 2;
  public $r = 3;
}

class D extends C {
  const D = "D::D";
  private $np = D::D;
  protected $nq = D::D;
  public $nr = D::D;
  private $p;
  protected $q;
  public $r;
}

class E {
  private $p;
  protected $q;
  public $r;

  <<__LateInit>> private $pl;
  <<__LateInit>> protected $ql;
  <<__LateInit>> public $rl;
}

<<__EntryPoint>>
function main_object_prop_array() :mixed{
var_dump(HH\object_prop_array(new C()));
var_dump(HH\object_prop_array(new D()));
var_dump(is_darray(HH\object_prop_array(new D())));

try {
  var_dump(HH\object_prop_array(new E(), true /* ignore_late_init */));
  var_dump(HH\object_prop_array(new E())); // should throw
} catch (Exception $e) {
  var_dump(get_class($e), $e->getMessage(), $e->getLine());
}
}
