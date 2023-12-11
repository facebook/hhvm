<?hh
<<__EntryPoint>> function main(): void {
$arr1 = vec['a','b','c'];
$arr2 = vec[];
$arr3 = dict[0 => 'c','key'=>'d'];
$arr4 = dict["a\0b"=>'e','key'=>'d', 0 => 'f'];

var_dump(in_array(123, $arr2));
var_dump(in_array(123, $arr1));
var_dump(array_search(123, $arr1));
var_dump(in_array('a', $arr1));
var_dump(array_search('a', $arr1));
var_dump(array_search('e', $arr4));
var_dump(array_search('d', $arr4));

echo "Done\n";
}
