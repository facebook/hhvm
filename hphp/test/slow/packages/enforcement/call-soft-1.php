<?hh

module a.b;

<<__EntryPoint>>
function main_call_soft_1() {
  $a = "A";
  $b = "b";
  $f = "f";

  $_ = new A();

  var_dump(A::b());
  var_dump(HH\dynamic_class_meth(A::class, $b)());

  var_dump($a::b());
  var_dump(HH\dynamic_class_meth($a, $b)());

  var_dump(f());
  var_dump(HH\dynamic_fun($f)());
}
