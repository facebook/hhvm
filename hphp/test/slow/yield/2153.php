<?hh


<<__EntryPoint>>
function main_2153() :mixed{
$a = function() {
 yield 1;
 yield 2;
 }
;
foreach ($a() as $v) {
 var_dump($v);
 }
}
