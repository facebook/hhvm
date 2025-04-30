<?hh

class C {}
class D {}

enum E : classname<C> {
  C = nameof C;
}

function ex(): void {
  $ss = nameof C;
  $s = __hhvm_intrinsics\launder_value($ss)."";
  $lc = C::class;
  $c = HH\classname_to_class($lc);

  var_dump(E::isValid($ss));
  var_dump(E::isValid($s));
  var_dump(E::isValid($lc));
  var_dump(E::isValid($c));
}

function noex(): void {
  $ss = nameof D;
  $s = __hhvm_intrinsics\launder_value($ss)."";
  $lc = D::class;
  $c = HH\classname_to_class($lc);

  var_dump(E::isValid($ss));
  var_dump(E::isValid($s));
  var_dump(E::isValid($lc));
  var_dump(E::isValid($c));
}


<<__EntryPoint>>
function main() {
  ex();
  noex();
}
