<?hh

class F {
 static function fruit() :AsyncGenerator<mixed,mixed,void>{
 yield 'apple';
 yield 'banana';
}
 }


<<__EntryPoint>>
function main_2142() :mixed{
foreach (F::fruit() as $fruit) {
 var_dump($fruit);
}
}
