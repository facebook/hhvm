<?hh <<__EntryPoint>> function main(): void {
$ar1 = dict["row1" => 2, "row2" => 1];
var_dump(array_multisort1(inout $ar1));
var_dump($ar1);

$ar1 = dict["row1" => 2, "row2" => 1];
var_dump(array_multisort1(inout $ar1));
var_dump($ar1);

$ar1 = dict["row1" => 2, "row2" => 1];
$args = vec[$ar1];
var_dump(call_user_func_array(array_multisort1<>, $args));
var_dump($ar1);
}
