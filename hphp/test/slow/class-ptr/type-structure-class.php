<?hh

class D {
  public function __construct() { echo "called\n"; }
}

class C {
  const type TGood = D;
  const type TBad = NoExist;
}

<<__EntryPoint>>
function f(): void {
  $ss = nameof C;
  $s = __hhvm_intrinsics\launder_value($ss)."";
  $lc = C::class;
  $c = HH\classname_to_class($lc);
  $o = new C();

  HH\type_structure_class($ss, 'TGood') |> new $$();
  HH\type_structure_class($s, 'TGood') |> new $$();
  HH\type_structure_class($lc, 'TGood') |> new $$();
  HH\type_structure_class($c, 'TGood') |> new $$();
  HH\type_structure_class($o, 'TGood') |> new $$();

  HH\type_structure_class(C::class, 'TBad');
}
