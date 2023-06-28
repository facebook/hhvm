<?hh
<<__DynamicallyCallable>>
function test(inout $a, $b) :mixed{
 $a = 'ok';
}

 <<__EntryPoint>>
function main_1183() :mixed{
 $a = 'test';
 $a(inout $a, 10);
 print $a;
}
