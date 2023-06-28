<?hh

function fruit() :AsyncGenerator<mixed,mixed,void>{
 $a = 123;
 yield $a;
 yield ++$a;
}


 <<__EntryPoint>>
function main_2144() :mixed{
foreach (fruit() as $fruit) {
 var_dump($fruit);
}
}
