<?hh


<<__EntryPoint>>
function main_1920() {
$abc = 123;
 $a = function () use ($abc) {
 var_dump($abc);
}
;
 $a();
}
