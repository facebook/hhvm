<?hh
 function bar() :mixed{
 return 123;
}


<<__EntryPoint>>
function main_1239() :mixed{
$a = bar();
 if ($a) {
   include '1239-1.inc';
 } else {
   include '1239-2.inc';
 }
 $obj = new foo();
}
