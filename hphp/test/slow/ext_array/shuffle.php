<?hh



<<__EntryPoint>>
function main_shuffle() :mixed{
$numbers = range(1, 4);
srand(5);
shuffle(inout $numbers);
$numArr = $numbers;
var_dump($numArr[0]);
var_dump($numArr[1]);
var_dump($numArr[2]);
var_dump($numArr[3]);
}
