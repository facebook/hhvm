<?hh
<<__DynamicallyCallable>>
function ref(inout $a, $b) :mixed{
  echo "$a $b";
}
<<__DynamicallyCallable>>
function val($a, $b)  :mixed{
  echo "$a $b";
}


<<__EntryPoint>>
function main_1098() :mixed{
$x = 0;
$foo0 = isset($g) ? ref<> : val<>;
$foo1 = isset($g) ? val<> : ref<>;
$t0 = $x; $x = 5; $t1 = $x;
$foo0($t0, $t1);
$x = 5; $t2 = $x;
$foo1(inout $x, $t2);
}
