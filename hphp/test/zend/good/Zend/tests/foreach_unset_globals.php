<?hh
<<__EntryPoint>> function main(): void {
$arr = dict["a" => 1, "b" => 2];
foreach ($arr as $key => $val) {
	\HH\global_unset($key);
}

var_dump($arr);
echo "Done\n";
}
