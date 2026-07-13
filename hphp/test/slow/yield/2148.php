<?hh

function nums() :AsyncGenerator<mixed,mixed,void>{
 $i = 0;
 while ($i < 3) { $t = $i; $i++; yield $t; }
}


 <<__EntryPoint>>
function main_2148() :mixed{
foreach (nums() as $num) {
 var_dump($num);
}
}
