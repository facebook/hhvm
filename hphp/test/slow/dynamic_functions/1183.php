<?hh
<<__DynamicallyCallable>>
function test(inout $a, $b) {
 $a = 'ok';
}

 <<__EntryPoint>>
function main_1183() {
 $a = 'test';
 $a(inout $a, 10);
 print $a;
}
