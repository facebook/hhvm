<?hh

function nums() {
 $i = 0;
 foo: switch ($i) {
 case 0: yield $i;
 $i = 1;
 case 999: yield $i;
 break;
 $i = -1;
 case 1: $i = 2;
 yield $i;
 yield break;
}
 goto foo;
}


 <<__EntryPoint>>
function main_2150() {
foreach (nums() as $num) {
 var_dump($num);
}
}
