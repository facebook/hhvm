<?hh


<<__EntryPoint>>
function main_2153() {
$a = function() {
 yield 1;
 yield 2;
 }
;
foreach ($a() as $v) {
 var_dump($v);
 }
}
