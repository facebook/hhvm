<?hh
<<__DynamicallyCallable>>
function test($a, $b) :mixed{
 print $a.$b;
}

 <<__EntryPoint>>
function main_1186() :mixed{
$a = 'test';
 $y = vec['k','q','q'];
 HH\dynamic_fun($a)('o',$y[0]);
}
