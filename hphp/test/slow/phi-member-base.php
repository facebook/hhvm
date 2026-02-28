<?hh

class C {}

class D {
  public function __construct(
    public mixed $x,
    public mixed $y,
  ) {}
}

<<__NEVER_INLINE>>
function set(D $d) :mixed{
  $result = vec[];
  $result[] = $d->x;
  $result[] = $d->y;
  return $result;
}

<<__EntryPoint>>
function main() :mixed{
  $arrays = vec[vec[new C()], dict[17 => new C()]];
  foreach ($arrays as $x) {
    var_dump(set(new D($x, true)));
  }
}
