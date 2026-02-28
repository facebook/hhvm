<?hh

function fruit() :AsyncGenerator<mixed,mixed,void>{
 yield 'apple';
 yield 'banana';
}


 <<__EntryPoint>>
function main_2141() :mixed{
foreach (fruit() as $fruit) {
 var_dump($fruit);
}
}
