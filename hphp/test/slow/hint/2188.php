<?hh

function f1(int $i = 1) :mixed{
 var_dump($i);
 }
function f2(float $d = 5.5) :mixed{
 var_dump($d);
 }
function f3(bool $b = true) :mixed{
 var_dump($b);
 }
function f4(string $s = 'hello') :mixed{
 var_dump($s);
 }

<<__EntryPoint>>
function main_2188() :mixed{
f1();
 f2();
 f3();
 f4();
}
