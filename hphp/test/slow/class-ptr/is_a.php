<?hh

class A {}

<<__EntryPoint>>
function main(): void {
  // Need these values outside of an array so they can be specialized
  $o = new A();
  $odesc = "object(A)   ";

  $s = nameof A;
  $sdesc = '"A"         ';

  $l = A::class;
  $ldesc = "lazyclass(A)";

  $c = HH\classname_to_class(A::class);
  $cdesc = "class(A)    ";

  echo "\$allow_string = false (implicit)\n\n";

  echo    "is_a($odesc, $sdesc) = ";
  var_dump(is_a($o,     $s,   ));
  echo    "is_a($sdesc, $sdesc) = ";
  var_dump(is_a($s,     $s,   ));
  echo    "is_a($ldesc, $sdesc) = ";
  var_dump(is_a($l,     $s,   ));
  echo    "is_a($cdesc, $sdesc) = ";
  var_dump(is_a($c,     $s,   ));

  echo    "is_a($odesc, $ldesc) = ";
  var_dump(is_a($o,     $l,   ));
  echo    "is_a($sdesc, $ldesc) = ";
  var_dump(is_a($s,     $l,   ));
  echo    "is_a($ldesc, $ldesc) = ";
  var_dump(is_a($l,     $l,   ));
  echo    "is_a($cdesc, $ldesc) = ";
  var_dump(is_a($c,     $l,   ));

  echo    "is_a($odesc, $cdesc) = ";
  var_dump(is_a($o,     $c,   ));
  echo    "is_a($sdesc, $cdesc) = ";
  var_dump(is_a($s,     $c,   ));
  echo    "is_a($ldesc, $cdesc) = ";
  var_dump(is_a($l,     $c,   ));
  echo    "is_a($cdesc, $cdesc) = ";
  var_dump(is_a($c,     $c,   ));

  echo "\n\$allow_string = false\n\n";

  echo    "is_a($odesc, $sdesc, false) = ";
  var_dump(is_a($o,     $s,     false));
  echo    "is_a($sdesc, $sdesc, false) = ";
  var_dump(is_a($s,     $s,     false));
  echo    "is_a($ldesc, $sdesc, false) = ";
  var_dump(is_a($l,     $s,     false));
  echo    "is_a($cdesc, $sdesc, false) = ";
  var_dump(is_a($c,     $s,     false));

  echo    "is_a($odesc, $ldesc, false) = ";
  var_dump(is_a($o,     $l,     false));
  echo    "is_a($sdesc, $ldesc, false) = ";
  var_dump(is_a($s,     $l,     false));
  echo    "is_a($ldesc, $ldesc, false) = ";
  var_dump(is_a($l,     $l,     false));
  echo    "is_a($cdesc, $ldesc, false) = ";
  var_dump(is_a($c,     $l,     false));

  echo    "is_a($odesc, $cdesc, false) = ";
  var_dump(is_a($o,     $c,     false));
  echo    "is_a($sdesc, $cdesc, false) = ";
  var_dump(is_a($s,     $c,     false));
  echo    "is_a($ldesc, $cdesc, false) = ";
  var_dump(is_a($l,     $c,     false));
  echo    "is_a($cdesc, $cdesc, false) = ";
  var_dump(is_a($c,     $c,     false));

  echo "\n\$allow_string = true\n\n";

  echo    "is_a($odesc, $sdesc, true) = ";
  var_dump(is_a($o,     $s,     true));
  echo    "is_a($sdesc, $sdesc, true) = ";
  var_dump(is_a($s,     $s,     true));
  echo    "is_a($ldesc, $sdesc, true) = ";
  var_dump(is_a($l,     $s,     true));
  echo    "is_a($cdesc, $sdesc, true) = ";
  var_dump(is_a($c,     $s,     true));

  echo    "is_a($odesc, $ldesc, true) = ";
  var_dump(is_a($o,     $l,     true));
  echo    "is_a($sdesc, $ldesc, true) = ";
  var_dump(is_a($s,     $l,     true));
  echo    "is_a($ldesc, $ldesc, true) = ";
  var_dump(is_a($l,     $l,     true));
  echo    "is_a($cdesc, $ldesc, true) = ";
  var_dump(is_a($c,     $l,     true));

  echo    "is_a($odesc, $cdesc, true) = ";
  var_dump(is_a($o,     $c,     true));
  echo    "is_a($sdesc, $cdesc, true) = ";
  var_dump(is_a($s,     $c,     true));
  echo    "is_a($ldesc, $cdesc, true) = ";
  var_dump(is_a($l,     $c,     true));
  echo    "is_a($cdesc, $cdesc, true) = ";
  var_dump(is_a($c,     $c,     true));

}
