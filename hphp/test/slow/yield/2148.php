<?hh

function nums() {
 $i = 0;
 while ($i < 3) yield $i++;
}


 <<__EntryPoint>>
function main_2148() {
foreach (nums() as $num) {
 var_dump($num);
}
}
