<?hh
 function bar() {
 return 123;
}


<<__EntryPoint>>
function main_1240() {
$a = bar();
 if ($a) {
   include '1240-1.inc';
 }
 else {
   include '1240-2.inc';
 }
 foo();
}
