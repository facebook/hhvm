<?hh

class C {
  const string K1 = "key1";
  const string K2 = "key2";
}

class D {
  const string K1 = "key3";
}

function f(): void {
  $d = dict[C::K1 => 42, D::K1 => "apple"];
  $d[C::K2] = 42.0;
  inspect($d);
}
