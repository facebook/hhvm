<?hh

class x {
  public $x0;
  public $y1;

  public function __toString() :mixed{
    return "it's ";
  }
}

function cyclic_prop_declared_setop_props() :mixed{
  $a = new x;
  $a->x0 = new x;
  $a->x0->y1 = $a->x0;
  var_dump($a);
  var_dump($a->x0->y1 .= "ok");
}

function cyclic_prop_nondeclared_setop_props() :mixed{
  $a = new x;
  $a->q0 = new x;
  $a->q0->r0 = 'a';
  $a->q0->r1 = $a->q0;
  $a->q0->y2 = $a->q0;
  $a->q0->r0 = 'b';
  var_dump($a);
  var_dump($a->q0->r1 .= "ok");
}
<<__EntryPoint>> function main(): void {
cyclic_prop_declared_setop_props();
cyclic_prop_nondeclared_setop_props();
}
