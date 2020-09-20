<?hh

function nums() {
 for ($i = 0;
 $i < 3;
 $i++) yield $i;
}


 <<__EntryPoint>>
function main_2147() {
foreach (nums() as $num) {
 var_dump($num);
}
}
