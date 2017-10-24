<?hh //strict

class P {
  public function __construct(public int $i, public string $s) { }
}
function baz():void {
  $vecpairs = vec[tuple(2,"a"), tuple(4,"b")];
  $dictpairs = dict[2 => tuple("a", 5), 3 => tuple("b", 7)];
  $p = new P(3, "c");
  $a = array(2,3,4);
  $sa = array("a");
  foreach ($vecpairs as list($p->i, $a[2])) {
    echo $p->i;
    echo $a[2];
  }

  foreach ($dictpairs as $i => list($sa[0], $p->i)) {
    echo $i;
    echo $sa[0];
    echo $p->i;
  }
}
baz();
