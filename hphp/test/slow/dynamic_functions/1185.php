<?hh
<<__DynamicallyCallable>>
function test($a, $b) :mixed{
 print $a.$b;
}

 <<__EntryPoint>>
function main_1185() :mixed{
$a = 'test';
 $y = 'kqq';
 $a('o',$y[0]);
}
