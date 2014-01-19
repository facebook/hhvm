<?hh

class C {
  public Vector<int> $x = Vector {};
}
$a = new C();
$b = new C();
$a->x->add(5);
$b->x->add(5);
echo $a->x->count(), "\n";

trait T {
  public Vector<int> $x = Vector {};
};
class D {
  use T;
}
$a = new D();
$b = new D();
$a->x->add(5);
$b->x->add(5);
echo $a->x->count(), "\n";

