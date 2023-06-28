<?hh

class C { function f() :mixed{} }

<<__EntryPoint>>
function main() :mixed{
  $m1 = meth_caller(C::class, 'f');
  $m2 = __hhvm_intrinsics\launder_value($m1);

  var_dump(HH\is_fun($m1));
  var_dump(HH\is_fun($m2));

  if (HH\is_fun($m1)) echo "oops\n";
  if (HH\is_fun($m2)) echo "oops\n";

  if (!HH\is_fun($m1)) echo "yes\n";
  if (!HH\is_fun($m2)) echo "yes\n";
}
