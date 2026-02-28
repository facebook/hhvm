<?hh


<<__EntryPoint>>
function main_508() :mixed{
$a = dict[1=>'main', 2=>'sub'];
$b = $a;
var_dump(array_pop(inout $b));
print_r($a);
var_dump(array_shift(inout $b));
print_r($a);
}
