<?hh

trait T {
 static function fruit() :AsyncGenerator<mixed,mixed,void>{
 yield 'apple';
 yield 'banana';
}
 }
class F {
 use T;
 }


<<__EntryPoint>>
function main_2068() :mixed{
foreach (F::fruit() as $fruit) {
 var_dump($fruit);
}
}
