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
$foo0($x, $x = 5);
$foo1(inout $x, $x = 5);
}
