<?hh
<<__EntryPoint>> function main(): void {
echo "-- (1)\n";
var_dump(filter_var_array(NULL));
var_dump(filter_var_array(vec[]));
var_dump(filter_var_array(dict[0 => 1,"blah" => "hoho"]));
var_dump(filter_var_array(vec[], -1));
var_dump(filter_var_array(vec[], 1000000));
var_dump(filter_var_array(vec[], ""));

echo "-- (2)\n";
var_dump(filter_var_array(dict[""=>""], -1));
var_dump(filter_var_array(dict[""=>""], 1000000));
var_dump(filter_var_array(dict[""=>""], ""));

echo "-- (3)\n";
var_dump(filter_var_array(dict["aaa"=>"bbb"], -1));
var_dump(filter_var_array(dict["aaa"=>"bbb"], 1000000));
var_dump(filter_var_array(dict["aaa"=>"bbb"], ""));

echo "-- (4)\n";
var_dump(filter_var_array(vec[], new stdClass));
var_dump(filter_var_array(vec[], vec[]));
var_dump(filter_var_array(vec[], dict["var_name"=>1]));
var_dump(filter_var_array(vec[], dict["var_name"=>-1]));
var_dump(filter_var_array(dict["var_name"=>""], dict["var_name"=>-1]));

echo "-- (5)\n";
var_dump(filter_var_array(dict["var_name"=>""], dict["var_name"=>-1, "asdas"=>"asdasd", "qwe"=>"rty", ""=>""]));
var_dump(filter_var_array(dict["asdas"=>"text"], dict["var_name"=>-1, "asdas"=>"asdasd", "qwe"=>"rty", ""=>""]));


$a = dict[""=>""]; $b = -1;
var_dump(filter_var_array($a, $b));
var_dump($a, $b);

$a = dict[""=>""]; $b = 100000;
var_dump(filter_var_array($a, $b));
var_dump($a, $b);

$a = dict[""=>""]; $b = "";
var_dump(filter_var_array($a, $b));
var_dump($a, $b);

echo "Done\n";
}
