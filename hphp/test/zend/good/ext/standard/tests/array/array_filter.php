<?hh
function odd($var)
{
   return($var & 1);
}

function even($var)
{
   return(!($var & 1));
}
<<__EntryPoint>> function main(): void {
$array1 = array("a"=>1, "b"=>2, "c"=>3, "d"=>4, "e"=>5);
$array2 = array(6, 7, 8, 9, 10, 11, 12, 0);
$array3 = array(TRUE, FALSE, NULL);

echo "Odd :\n";
var_dump(array_filter($array1, fun("odd")));
var_dump(array_filter($array2, fun("odd")));
var_dump(array_filter($array3, fun("odd")));
echo "Even:\n";
var_dump(array_filter($array1, fun("even")));
var_dump(array_filter($array2, fun("even")));
var_dump(array_filter($array3, fun("even")));

var_dump(array_filter(array()));
var_dump(array_filter(array(), array()));
var_dump(array_filter($array1, 1));

echo '== DONE ==';
}
