<?hh

class C {
  public static vec<int> $p = vec[];
}

class D extends C {
}

<<__EntryPoint>>
function main(): void {
  C::$p[] = 1;
  D::$p[] = 2;
  var_dump(C::$p); // vec[1,2]
  var_dump(D::$p); // vec[1,2]
}
