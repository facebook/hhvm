<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

class Harmless {
  public function __construct()[]: void {}
}

class C {
  public ?int $i = null;

  public function harmless()[]: void {}
  public static function staticHarmless()[]: void {}
}

function harmless1(C $c)[]: void {}
function harmless2(C $c)[globals]: void {}

function takes_int(int $i)[]: void {}

function refine_ok(
  C $c,
  ((function()[]: void) &  (function()[write_props]: void)) $harmless_intersection1,
  ((function()[policied]: void) & (function()[globals]: void)) $harmless_intersection2,
): void {
  if ($c->i is nonnull) {
    harmless1($c);
    takes_int($c->i);

    harmless2($c);
    takes_int($c->i);

    Shapes::idx(shape('key' => 42), 'key');
    takes_int($c->i);

    $dict = dict[];
    isset($dict['key']);
    takes_int($c->i);

    is_null($c);
    takes_int($c->i);

    $harmless_union = (1 === 2) ? ()[] ==> {} : ()[globals] ==> {};
    $harmless_union();
    takes_int($c->i);

    $harmless_intersection1();
    takes_int($c->i);

    $harmless_intersection2();
    takes_int($c->i);

    new Harmless();
    takes_int($c->i);

    array_map($x ==> $x, vec[]);
    takes_int($c->i);

    $c->harmless();
    takes_int($c->i);

    C::staticHarmless();
    takes_int($c->i);
  }
}
