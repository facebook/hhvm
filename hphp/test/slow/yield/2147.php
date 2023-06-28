<?hh

function nums() :AsyncGenerator<mixed,mixed,void>{
 for ($i = 0;
 $i < 3;
 $i++) yield $i;
}


 <<__EntryPoint>>
function main_2147() :mixed{
foreach (nums() as $num) {
 var_dump($num);
}
}
