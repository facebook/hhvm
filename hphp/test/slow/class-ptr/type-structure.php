<?hh

class X {}

class C {
  const type T = X;
}

<<__EntryPoint>>
function main(): void {
  $ss = nameof C;
  $s = __hhvm_intrinsics\launder_value($ss)."";
  $lc = C::class;
  $c = HH\classname_to_class($lc);
  $o = new C();
  $i = 4;

  var_dump(type_structure($ss, 'T')['classname']); // log
  var_dump(type_structure($s, 'T')['classname']); // log
  var_dump(type_structure($lc, 'T')['classname']);
  var_dump(type_structure($c, 'T')['classname']);
  var_dump(type_structure($o, 'T')['classname']);

  // document strange fatal behavior
  type_structure($i, 'T');
}
