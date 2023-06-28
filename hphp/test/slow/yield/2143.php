<?hh

class F {
 function fruit() :AsyncGenerator<mixed,mixed,void>{
 yield 'apple';
 yield 'banana';
}
 }


<<__EntryPoint>>
function main_2143() :mixed{
$f = new F;
 foreach ($f->fruit() as $fruit) {
 var_dump($fruit);
}
}
