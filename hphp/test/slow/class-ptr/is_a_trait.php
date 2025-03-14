<?hh

<<__DynamicallyReferenced>> interface I {}
<<__DynamicallyReferenced>> trait T implements I {}

<<__DynamicallyReferenced>> trait T1 {}
<<__DynamicallyReferenced>> trait T2 { use T1; }

class A { use T; }
class B { use T1; }
class C { use T2; }

function traits(): void {
  $st = nameof T;
  $lt = T::class;
  $ct = HH\classname_to_class(nameof T);

  $si = nameof I;
  $li = I::class;
  $ci = HH\classname_to_class(nameof I);

  echo "==> is_a - T, T\n";
  var_dump(is_a($st, $st, true));
  var_dump(is_a($st, $lt, true));
  var_dump(is_a($st, $ct, true));
  var_dump(is_a($lt, $st, true));
  var_dump(is_a($lt, $lt, true));
  var_dump(is_a($lt, $ct, true));
  var_dump(is_a($ct, $st, true));
  var_dump(is_a($ct, $lt, true));
  var_dump(is_a($ct, $ct, true));

  echo "==> is_a - T, I\n";
  var_dump(is_a($st, $si, true));
  var_dump(is_a($st, $li, true));
  var_dump(is_a($st, $ci, true));
  var_dump(is_a($lt, $si, true));
  var_dump(is_a($lt, $li, true));
  var_dump(is_a($lt, $ci, true));
  var_dump(is_a($ct, $si, true));
  var_dump(is_a($ct, $li, true));
  var_dump(is_a($ct, $ci, true));

  $st1 = nameof T1;
  $lt1 = T1::class;
  $ct1 = HH\classname_to_class(nameof T1);

  $st2 = nameof T2;
  $lt2 = T2::class;
  $ct2 = HH\classname_to_class(nameof T2);
  echo "==> is_a - T2, T1\n";
  var_dump(is_a($st2, $st1, true));
  var_dump(is_a($st2, $lt1, true));
  var_dump(is_a($st2, $ct1, true));
  var_dump(is_a($lt2, $st1, true));
  var_dump(is_a($lt2, $lt1, true));
  var_dump(is_a($lt2, $ct1, true));
  var_dump(is_a($ct2, $st1, true));
  var_dump(is_a($ct2, $lt1, true));
  var_dump(is_a($ct2, $ct1, true));
}

function objects(): void {
  echo "==> is_a - object(A), T\n";
  var_dump(is_a(new A(), nameof T, true));
  var_dump(is_a(new A(), T::class, true));
  var_dump(is_a(new A(), HH\classname_to_class(nameof T), true));

  echo "==> is_a - object(A), I === true\n";
  var_dump(is_a(new A(), nameof I, true));
  var_dump(is_a(new A(), I::class, true));
  var_dump(is_a(new A(), HH\classname_to_class(nameof I), true));

  echo "==> is_a - object(B), T1\n";
  var_dump(is_a(new B(), nameof T1, true));
  var_dump(is_a(new B(), T1::class, true));
  var_dump(is_a(new B(), HH\classname_to_class(nameof T1), true));

  echo "==> is_a - object(C), T2\n";
  var_dump(is_a(new C(), nameof T2, true));
  var_dump(is_a(new C(), T2::class, true));
  var_dump(is_a(new C(), HH\classname_to_class(nameof T2), true));

  echo "==> is_a - object(C), T1\n";
  var_dump(is_a(new C(), nameof T1, true));
  var_dump(is_a(new C(), T1::class, true));
  var_dump(is_a(new C(), HH\classname_to_class(nameof T1), true));
}

<<__EntryPoint>>
function main(): void {
  traits();

  objects();
}
