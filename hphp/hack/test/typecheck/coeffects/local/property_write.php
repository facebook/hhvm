<?hh

class X {
  public function __construct(
    public int $integer,
    public string $str,
    public Vector<int> $vector,
    public varray<int> $varr,
  ) {}
}

function ok(X $x)[write_props]: void {
  $x->integer = 5;
  $x->str .= 0;
  $x->vector[] = 1;
  $x->varr[] = 2;
}

function fail(X $x)[]: void {
  $x->integer = 5; // ERROR
  $x->str .= 0; // ERROR
  $x->vector[] = 1; // ERROR
  $x->varr[] = 2;
}
