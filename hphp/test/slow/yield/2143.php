<?hh

class F {
 function fruit() {
 yield 'apple';
 yield 'banana';
}
 }


<<__EntryPoint>>
function main_2143() {
$f = new F;
 foreach ($f->fruit() as $fruit) {
 var_dump($fruit);
}
}
