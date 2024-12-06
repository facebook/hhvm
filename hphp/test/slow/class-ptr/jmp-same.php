<?hh

class C {}
class D {}

class Check {
  public static bool $should = true;
  public static function t(): void { // then
    invariant(self::$should, "if branch should not execute");
  }
  public static function e(): void { // else
    invariant(!self::$should, "else branch should not execute");
  }
}

function dstr(string $s): string {
  return __hhvm_intrinsics\launder_value($s);
}
<<__EntryPoint>>
function main(): void {
  // HHBBC types annotated

  $c_str = nameof C; // Str="C"
  $c_dstr = dstr(nameof C); // Str
  $c_lcls = C::class; // LazyCls="C"
  // skipping over $c_dlcls: LazyCls because it's non-trivial to construct
  $c_cls = HH\classname_to_class(C::class); // Cls=C
  $c_dcls = HH\classname_to_class(__hhvm_intrinsics\launder_value(C::class)); // Cls
  $c_cls2 = HH\get_class_from_object(new C());
  $c_dcls2 = HH\get_class_from_object(__hhvm_intrinsics\launder_value(new C()));

  $d_str = nameof D; // Str="D"
  $d_dstr = dstr(nameof D); // Str
  $d_lcls = D::class; // LazyCls="D"
  $d_cls = HH\classname_to_class(D::class); // Cls=D
  $d_dcls = HH\classname_to_class(__hhvm_intrinsics\launder_value(D::class)); // Cls
  $d_cls2 = HH\get_class_from_object(new D());
  $d_dcls2 = HH\get_class_from_object(__hhvm_intrinsics\launder_value(new D()));

  /* JmpZ cases */

  echo "Checking C === C\n";
  Check::$should = true;
  if ($c_str  === $c_str)  { Check::t();  } else { Check::e(); }
  if ($c_str  === $c_dstr) { Check::t();  } else { Check::e(); }
  if ($c_str  === $c_lcls) { Check::t();  } else { Check::e(); }
  if ($c_str  === $c_cls)  { Check::t();  } else { Check::e(); }
  if ($c_str  === $c_dcls) { Check::t();  } else { Check::e(); }
  if ($c_str  === $c_cls2) { Check::t();  } else { Check::e(); }
  if ($c_str  === $c_dcls2){ Check::t();  } else { Check::e(); }

  if ($c_dstr === $c_str)  { Check::t();  } else { Check::e(); }
  if ($c_dstr === $c_dstr) { Check::t();  } else { Check::e(); }
  if ($c_dstr === $c_lcls) { Check::t();  } else { Check::e(); }
  if ($c_dstr === $c_cls)  { Check::t();  } else { Check::e(); }
  if ($c_dstr === $c_dcls) { Check::t();  } else { Check::e(); }
  if ($c_dstr === $c_cls2) { Check::t();  } else { Check::e(); }
  if ($c_dstr === $c_dcls2){ Check::t();  } else { Check::e(); }

  if ($c_lcls === $c_str)  { Check::t();  } else { Check::e(); }
  if ($c_lcls === $c_dstr) { Check::t();  } else { Check::e(); }
  if ($c_lcls === $c_lcls) { Check::t();  } else { Check::e(); }
  if ($c_lcls === $c_cls)  { Check::t();  } else { Check::e(); }
  if ($c_lcls === $c_dcls) { Check::t();  } else { Check::e(); }
  if ($c_lcls === $c_cls2) { Check::t();  } else { Check::e(); }
  if ($c_lcls === $c_dcls2){ Check::t();  } else { Check::e(); }

  if ($c_cls  === $c_str)  { Check::t();  } else { Check::e(); }
  if ($c_cls  === $c_dstr) { Check::t();  } else { Check::e(); }
  if ($c_cls  === $c_lcls) { Check::t();  } else { Check::e(); }
  if ($c_cls  === $c_cls)  { Check::t();  } else { Check::e(); }
  if ($c_cls  === $c_dcls) { Check::t();  } else { Check::e(); }
  if ($c_cls  === $c_cls2) { Check::t();  } else { Check::e(); }
  if ($c_cls  === $c_dcls2){ Check::t();  } else { Check::e(); }

  if ($c_dcls === $c_str)  { Check::t();  } else { Check::e(); }
  if ($c_dcls === $c_dstr) { Check::t();  } else { Check::e(); }
  if ($c_dcls === $c_lcls) { Check::t();  } else { Check::e(); }
  if ($c_dcls === $c_cls)  { Check::t();  } else { Check::e(); }
  if ($c_dcls === $c_dcls) { Check::t();  } else { Check::e(); }
  if ($c_dcls === $c_cls2) { Check::t();  } else { Check::e(); }
  if ($c_dcls === $c_dcls2){ Check::t();  } else { Check::e(); }

  if ($c_cls2 === $c_str)  { Check::t();  } else { Check::e(); }
  if ($c_cls2 === $c_dstr) { Check::t();  } else { Check::e(); }
  if ($c_cls2 === $c_lcls) { Check::t();  } else { Check::e(); }
  if ($c_cls2 === $c_cls)  { Check::t();  } else { Check::e(); }
  if ($c_cls2 === $c_dcls) { Check::t();  } else { Check::e(); }
  if ($c_cls2 === $c_cls2) { Check::t();  } else { Check::e(); }
  if ($c_cls2 === $c_dcls2){ Check::t();  } else { Check::e(); }

  if ($c_dcls2=== $c_str)  { Check::t();  } else { Check::e(); }
  if ($c_dcls2=== $c_dstr) { Check::t();  } else { Check::e(); }
  if ($c_dcls2=== $c_lcls) { Check::t();  } else { Check::e(); }
  if ($c_dcls2=== $c_cls)  { Check::t();  } else { Check::e(); }
  if ($c_dcls2=== $c_dcls) { Check::t();  } else { Check::e(); }
  if ($c_dcls2=== $c_cls2) { Check::t();  } else { Check::e(); }
  if ($c_dcls2=== $c_dcls2){ Check::t();  } else { Check::e(); }

  echo "Checking C === D\n";
  Check::$should = false;
  if ($c_str  === $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_str  === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_str  === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_str  === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_str  === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_str  === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_str  === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_dstr === $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_dstr === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_dstr === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_dstr === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_dstr === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_dstr === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_dstr === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_lcls === $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_lcls === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_lcls === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_lcls === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_lcls === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_lcls === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_lcls === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_cls  === $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_cls  === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_cls  === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_cls  === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_cls  === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_cls  === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_cls  === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_dcls === $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_dcls === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_dcls === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_dcls === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_dcls === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_cls2 === $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_cls2 === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_cls2 === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_cls2 === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_cls2 === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_cls2 === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_cls2 === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_dcls2=== $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_dcls2=== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_dcls2=== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls2=== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_dcls2=== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls2=== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_dcls2=== $d_dcls2){ Check::t(); } else { Check::e(); }


  echo "Checking D === C\n";
  Check::$should = false;
  if ($d_str  === $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_str  === $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_str  === $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_str  === $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_str  === $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_str  === $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_str  === $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dstr === $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_dstr === $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dstr === $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dstr === $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dstr === $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dstr === $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dstr === $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_lcls === $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_lcls === $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_lcls === $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_lcls === $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_lcls === $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_lcls === $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_lcls === $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_cls  === $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_cls  === $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_cls  === $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_cls  === $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_cls  === $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_cls  === $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_cls  === $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dcls === $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_dcls === $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dcls === $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls === $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dcls === $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls === $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dcls === $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_cls2 === $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dcls2=== $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $c_dcls2){ Check::t(); } else { Check::e(); }

  echo "Checking D === D\n";
  Check::$should = true;
  if ($d_str  === $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_str  === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_str  === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_str  === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_str  === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_str  === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_str  === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dstr === $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_dstr === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dstr === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dstr === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dstr === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dstr === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dstr === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_lcls === $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_lcls === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_lcls === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_lcls === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_lcls === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_lcls === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_lcls === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_cls  === $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_cls  === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_cls  === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_cls  === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_cls  === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_cls  === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_cls  === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dcls === $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_dcls === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dcls === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dcls === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dcls === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_cls2 === $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_cls2 === $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dcls2=== $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dcls2=== $d_dcls2){ Check::t(); } else { Check::e(); }

  /* JmpNZ cases */

  echo "Checking C !== C\n";
  Check::$should = false;
  if ($c_str  !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($c_str  !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($c_str  !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($c_str  !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($c_str  !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($c_str  !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($c_str  !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_dstr !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_lcls !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_cls  !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_dcls !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_cls2 !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_dcls2!== $c_str)  { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $c_dcls2){ Check::t(); } else { Check::e(); }

  echo "Checking C !== D\n";
  Check::$should = true;
  if ($c_str  !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_str  !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_str  !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_str  !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_str  !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_str  !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_str  !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_dstr !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_dstr !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_lcls !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_lcls !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_cls  !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_cls  !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_dcls !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_dcls !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_cls2 !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_cls2 !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($c_dcls2!== $d_str)  { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($c_dcls2!== $d_dcls2){ Check::t(); } else { Check::e(); }

  echo "Checking D !== C\n";
  Check::$should = true;
  if ($d_str  !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_str  !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_str  !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_str  !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_str  !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_str  !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_str  !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dstr !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_lcls !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_cls  !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dcls !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_cls2 !== $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $c_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dcls2!== $c_str)  { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $c_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $c_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $c_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $c_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $c_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $c_dcls2){ Check::t(); } else { Check::e(); }

  echo "Checking D !== D\n";
  Check::$should = false;
  if ($d_str  !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_str  !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_str  !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_str  !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_str  !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_str  !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_str  !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dstr !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dstr !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_lcls !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_lcls !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_cls  !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_cls  !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dcls !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dcls !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_cls2 !== $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_cls2 !== $d_dcls2){ Check::t(); } else { Check::e(); }

  if ($d_dcls2!== $d_str)  { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $d_dstr) { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $d_lcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $d_cls)  { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $d_dcls) { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $d_cls2) { Check::t(); } else { Check::e(); }
  if ($d_dcls2!== $d_dcls2){ Check::t(); } else { Check::e(); }
}
