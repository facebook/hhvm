<?hh

class C {
  public Vector<int> $x = Vector {};
}

trait T {
  public Vector<int> $x = Vector {};
}class D {
  use T;
}


<<__EntryPoint>>
function main_prop_init_trait_bug() :mixed{
$a = new C();
$b = new C();
$a->x->add(5);
$b->x->add(5);
echo $a->x->count(), "\n";
;
$a = new D();
$b = new D();
$a->x->add(5);
$b->x->add(5);
echo $a->x->count(), "\n";
}
