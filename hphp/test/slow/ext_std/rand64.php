<?hh

<<__EntryPoint>>
function main_rand64() :mixed{
srand(0);
echo rand(0, PHP_INT_MAX)."\n";
}
