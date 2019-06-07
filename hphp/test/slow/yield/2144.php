<?hh

function fruit() {
 $a = 123;
 yield $a;
 yield ++$a;
}


 <<__EntryPoint>>
function main_2144() {
foreach (fruit() as $fruit) {
 var_dump($fruit);
}
}
