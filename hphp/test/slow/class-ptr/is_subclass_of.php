<?hh

class A {}
class B extends A {}

function subclass(): void {
  // Need these values outside of an array so they can be specialized
  $o = new B();
  $odesc = "object(A)   ";

  $s = nameof B;
  $sdesc = '"B"         ';

  $l = B::class;
  $ldesc = "lazyclass(B)";

  $c = HH\classname_to_class(B::class);
  $cdesc = "class(B)    ";

  $s2 = nameof A;
  $sdesc2 = '"A"         ';

  $l2 = A::class;
  $ldesc2 = "lazyclass(A)";

  $c2 = HH\classname_to_class(A::class);
  $cdesc2 = "class(A)    ";

  echo "\$allow_string = true (implicit)\n\n";

  echo    "is_subclass_of($odesc, $sdesc2) = ";
  var_dump(is_subclass_of($o,     $s2,   ));
  echo    "is_subclass_of($sdesc, $sdesc2) = ";
  var_dump(is_subclass_of($s,     $s2,   ));
  echo    "is_subclass_of($ldesc, $sdesc2) = ";
  var_dump(is_subclass_of($l,     $s2,   ));
  echo    "is_subclass_of($cdesc, $sdesc2) = ";
  var_dump(is_subclass_of($c,     $s2,   ));

  echo    "is_subclass_of($odesc, $ldesc2) = ";
  var_dump(is_subclass_of($o,     $l2,   ));
  echo    "is_subclass_of($sdesc, $ldesc2) = ";
  var_dump(is_subclass_of($s,     $l2,   ));
  echo    "is_subclass_of($ldesc, $ldesc2) = ";
  var_dump(is_subclass_of($l,     $l2,   ));
  echo    "is_subclass_of($cdesc, $ldesc2) = ";
  var_dump(is_subclass_of($c,     $l2,   ));

  echo    "is_subclass_of($odesc, $cdesc2) = ";
  var_dump(is_subclass_of($o,     $c2,   ));
  echo    "is_subclass_of($sdesc, $cdesc2) = ";
  var_dump(is_subclass_of($s,     $c2,   ));
  echo    "is_subclass_of($ldesc, $cdesc2) = ";
  var_dump(is_subclass_of($l,     $c2,   ));
  echo    "is_subclass_of($cdesc, $cdesc2) = ";
  var_dump(is_subclass_of($c,     $c2,   ));

  echo "\n\$allow_string = false\n\n";

  echo    "is_subclass_of($odesc, $sdesc2, false) = ";
  var_dump(is_subclass_of($o,     $s2,     false));
  echo    "is_subclass_of($sdesc, $sdesc2, false) = ";
  var_dump(is_subclass_of($s,     $s2,     false));
  echo    "is_subclass_of($ldesc, $sdesc2, false) = ";
  var_dump(is_subclass_of($l,     $s2,     false));
  echo    "is_subclass_of($cdesc, $sdesc2, false) = ";
  var_dump(is_subclass_of($c,     $s2,     false));

  echo    "is_subclass_of($odesc, $ldesc2, false) = ";
  var_dump(is_subclass_of($o,     $l2,     false));
  echo    "is_subclass_of($sdesc, $ldesc2, false) = ";
  var_dump(is_subclass_of($s,     $l2,     false));
  echo    "is_subclass_of($ldesc, $ldesc2, false) = ";
  var_dump(is_subclass_of($l,     $l2,     false));
  echo    "is_subclass_of($cdesc, $ldesc2, false) = ";
  var_dump(is_subclass_of($c,     $l2,     false));

  echo    "is_subclass_of($odesc, $cdesc2, false) = ";
  var_dump(is_subclass_of($o,     $c2,     false));
  echo    "is_subclass_of($sdesc, $cdesc2, false) = ";
  var_dump(is_subclass_of($s,     $c2,     false));
  echo    "is_subclass_of($ldesc, $cdesc2, false) = ";
  var_dump(is_subclass_of($l,     $c2,     false));
  echo    "is_subclass_of($cdesc, $cdesc2, false) = ";
  var_dump(is_subclass_of($c,     $c2,     false));

  echo "\n\$allow_string = true\n\n";

  echo    "is_subclass_of($odesc, $sdesc2, true) = ";
  var_dump(is_subclass_of($o,     $s2,     true));
  echo    "is_subclass_of($sdesc, $sdesc2, true) = ";
  var_dump(is_subclass_of($s,     $s2,     true));
  echo    "is_subclass_of($ldesc, $sdesc2, true) = ";
  var_dump(is_subclass_of($l,     $s2,     true));
  echo    "is_subclass_of($cdesc, $sdesc2, true) = ";
  var_dump(is_subclass_of($c,     $s2,     true));

  echo    "is_subclass_of($odesc, $ldesc2, true) = ";
  var_dump(is_subclass_of($o,     $l2,     true));
  echo    "is_subclass_of($sdesc, $ldesc2, true) = ";
  var_dump(is_subclass_of($s,     $l2,     true));
  echo    "is_subclass_of($ldesc, $ldesc2, true) = ";
  var_dump(is_subclass_of($l,     $l2,     true));
  echo    "is_subclass_of($cdesc, $ldesc2, true) = ";
  var_dump(is_subclass_of($c,     $l2,     true));

  echo    "is_subclass_of($odesc, $cdesc2, true) = ";
  var_dump(is_subclass_of($o,     $c2,     true));
  echo    "is_subclass_of($sdesc, $cdesc2, true) = ";
  var_dump(is_subclass_of($s,     $c2,     true));
  echo    "is_subclass_of($ldesc, $cdesc2, true) = ";
  var_dump(is_subclass_of($l,     $c2,     true));
  echo    "is_subclass_of($cdesc, $cdesc2, true) = ";
  var_dump(is_subclass_of($c,     $c2,     true));
}

function not_subclass(): void {
  // Need these values outside of an array so they can be specialized
  $o = new A();
  $odesc = "object(A)   ";

  $s = nameof A;
  $sdesc = '"A"         ';

  $l = A::class;
  $ldesc = "lazyclass(A)";

  $c = HH\classname_to_class(A::class);
  $cdesc = "class(A)    ";

  echo "\$allow_string = true (implicit)\n\n";

  echo    "is_subclass_of($odesc, $sdesc) = ";
  var_dump(is_subclass_of($o,     $s,   ));
  echo    "is_subclass_of($sdesc, $sdesc) = ";
  var_dump(is_subclass_of($s,     $s,   ));
  echo    "is_subclass_of($ldesc, $sdesc) = ";
  var_dump(is_subclass_of($l,     $s,   ));
  echo    "is_subclass_of($cdesc, $sdesc) = ";
  var_dump(is_subclass_of($c,     $s,   ));

  echo    "is_subclass_of($odesc, $ldesc) = ";
  var_dump(is_subclass_of($o,     $l,   ));
  echo    "is_subclass_of($sdesc, $ldesc) = ";
  var_dump(is_subclass_of($s,     $l,   ));
  echo    "is_subclass_of($ldesc, $ldesc) = ";
  var_dump(is_subclass_of($l,     $l,   ));
  echo    "is_subclass_of($cdesc, $ldesc) = ";
  var_dump(is_subclass_of($c,     $l,   ));

  echo    "is_subclass_of($odesc, $cdesc) = ";
  var_dump(is_subclass_of($o,     $c,   ));
  echo    "is_subclass_of($sdesc, $cdesc) = ";
  var_dump(is_subclass_of($s,     $c,   ));
  echo    "is_subclass_of($ldesc, $cdesc) = ";
  var_dump(is_subclass_of($l,     $c,   ));
  echo    "is_subclass_of($cdesc, $cdesc) = ";
  var_dump(is_subclass_of($c,     $c,   ));

  echo "\n\$allow_string = false\n\n";

  echo    "is_subclass_of($odesc, $sdesc, false) = ";
  var_dump(is_subclass_of($o,     $s,     false));
  echo    "is_subclass_of($sdesc, $sdesc, false) = ";
  var_dump(is_subclass_of($s,     $s,     false));
  echo    "is_subclass_of($ldesc, $sdesc, false) = ";
  var_dump(is_subclass_of($l,     $s,     false));
  echo    "is_subclass_of($cdesc, $sdesc, false) = ";
  var_dump(is_subclass_of($c,     $s,     false));

  echo    "is_subclass_of($odesc, $ldesc, false) = ";
  var_dump(is_subclass_of($o,     $l,     false));
  echo    "is_subclass_of($sdesc, $ldesc, false) = ";
  var_dump(is_subclass_of($s,     $l,     false));
  echo    "is_subclass_of($ldesc, $ldesc, false) = ";
  var_dump(is_subclass_of($l,     $l,     false));
  echo    "is_subclass_of($cdesc, $ldesc, false) = ";
  var_dump(is_subclass_of($c,     $l,     false));

  echo    "is_subclass_of($odesc, $cdesc, false) = ";
  var_dump(is_subclass_of($o,     $c,     false));
  echo    "is_subclass_of($sdesc, $cdesc, false) = ";
  var_dump(is_subclass_of($s,     $c,     false));
  echo    "is_subclass_of($ldesc, $cdesc, false) = ";
  var_dump(is_subclass_of($l,     $c,     false));
  echo    "is_subclass_of($cdesc, $cdesc, false) = ";
  var_dump(is_subclass_of($c,     $c,     false));

  echo "\n\$allow_string = true\n\n";

  echo    "is_subclass_of($odesc, $sdesc, true) = ";
  var_dump(is_subclass_of($o,     $s,     true));
  echo    "is_subclass_of($sdesc, $sdesc, true) = ";
  var_dump(is_subclass_of($s,     $s,     true));
  echo    "is_subclass_of($ldesc, $sdesc, true) = ";
  var_dump(is_subclass_of($l,     $s,     true));
  echo    "is_subclass_of($cdesc, $sdesc, true) = ";
  var_dump(is_subclass_of($c,     $s,     true));

  echo    "is_subclass_of($odesc, $ldesc, true) = ";
  var_dump(is_subclass_of($o,     $l,     true));
  echo    "is_subclass_of($sdesc, $ldesc, true) = ";
  var_dump(is_subclass_of($s,     $l,     true));
  echo    "is_subclass_of($ldesc, $ldesc, true) = ";
  var_dump(is_subclass_of($l,     $l,     true));
  echo    "is_subclass_of($cdesc, $ldesc, true) = ";
  var_dump(is_subclass_of($c,     $l,     true));

  echo    "is_subclass_of($odesc, $cdesc, true) = ";
  var_dump(is_subclass_of($o,     $c,     true));
  echo    "is_subclass_of($sdesc, $cdesc, true) = ";
  var_dump(is_subclass_of($s,     $c,     true));
  echo    "is_subclass_of($ldesc, $cdesc, true) = ";
  var_dump(is_subclass_of($l,     $c,     true));
  echo    "is_subclass_of($cdesc, $cdesc, true) = ";
  var_dump(is_subclass_of($c,     $c,     true));
}

<<__EntryPoint>>
function main(): void {
  echo "Subclass cases\n";
  echo "==============\n\n";

  subclass();

  echo "\n\nEqual class cases\n";
  echo "=================\n\n";

  not_subclass();
}
