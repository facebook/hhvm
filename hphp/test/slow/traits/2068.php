<?hh

trait T {
 static function fruit() {
 yield 'apple';
 yield 'banana';
}
 }
class F {
 use T;
 }


<<__EntryPoint>>
function main_2068() {
foreach (F::fruit() as $fruit) {
 var_dump($fruit);
}
}
