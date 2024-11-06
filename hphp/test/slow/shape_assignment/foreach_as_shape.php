<?hh
<<file:__EnableUnstableFeatures('shape_destructure')>>

class P {
  public function __construct(public int $i, public string $s) { }
}
function baz():void {
  $vecpairs = vec[shape('a' => 2,'b' => "a"), shape('a' => 4,'b' => "b")];
  $dictpairs = dict[2 => shape('a' => "a", 'b' => 5), 3 => shape('a' => "b", 'b' => 7)];
  $p = new P(3, "c");
  $a = vec[2,3,4];
  $sa = vec["a"];
  foreach ($vecpairs as shape('a' => $p->i, 'b' => $a[2])) {
    echo $p->i;
    echo $a[2];
  }

  foreach ($dictpairs as $i => shape('a' => $sa[0], 'b' => $p->i)) {
    echo $i;
    echo $sa[0];
    echo $p->i;
  }
}

<<__EntryPoint>>
function main_foreach_as_list() :mixed{
baz();
}
