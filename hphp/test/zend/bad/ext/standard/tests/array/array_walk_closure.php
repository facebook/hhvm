<?php

var_dump(array_walk());

$ar = false;
var_dump(array_walk($ar, $ar));

$ar = NULL;
var_dump(array_walk($ar, $ar));

$ar = ["one" => 1, "two"=>2, "three" => 3];
var_dump(array_walk($ar, function(){ var_dump(func_get_args());}));

echo "\nclosure with array\n";
$ar = ["one" => 1, "two"=>2, "three" => 3];
$user_data = ["sum" => 42];
$func = function($value, $key, &$udata) {
	var_dump($udata);
	$udata["sum"] += $value;
};

var_dump(array_walk($ar, $func, $user_data));
echo "End result:";
var_dump($user_data["sum"]);

echo "\nclosure with use\n";
$ar = ["one" => 1, "two"=>2, "three" => 3];
$user_data = ["sum" => 42];
$func = function($value, $key) use (&$user_data) {
	var_dump($user_data);
	$user_data["sum"] += $value;
};

var_dump(array_walk($ar, $func, $user_data));
echo "End result:";
var_dump($user_data["sum"]);


echo "\nclosure with object\n";
$ar = ["one" => 1, "two"=>2, "three" => 3];
$user_data = (object)["sum" => 42];
$func = function($value, $key, &$udata) {
	var_dump($udata);
	$udata->sum += $value;
};

var_dump(array_walk($ar, $func, $user_data));
echo "End result:";
var_dump($user_data->sum);



echo "\nfunction with object\n";
function sum_it_up_object($value, $key, $udata)
{
	var_dump($udata);
	$udata->sum += $value;
}

$ar = ["one" => 1, "two"=>2, "three" => 3];
$user_data = (object)["sum" => 42];

var_dump(array_walk($ar, "sum_it_up_object", $user_data));
echo "End result:";
var_dump($user_data->sum);


echo "\nfunction with array\n";
function sum_it_up_array($value, $key, $udata)
{
	var_dump($udata);
	$udata['sum'] += $value;
}

$ar = ["one" => 1, "two"=>2, "three" => 3];
$user_data = ["sum" => 42];

var_dump(array_walk($ar, "sum_it_up_array", $user_data));
echo "End result:";
var_dump($user_data['sum']);

echo "\nclosure and exception\n";
$ar = ["one" => 1, "two"=>2, "three" => 3];
try {
	var_dump(array_walk($ar, function($v, $k) { if ($v == 2) throw new Exception; } ));
} catch (Exception $e) {
	var_dump($e->getTrace());
}


echo "Done\n";
?>