<?hh
function foo(Vector<int> $a) :mixed{
}
function bar(string $x) :mixed{
 echo $x . "\n";
 }

<<__EntryPoint>>
function main_hh_2() :mixed{
bar("Done");
}
