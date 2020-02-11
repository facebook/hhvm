<?hh
<<__EntryPoint>> function main(): void {
echo "-- (1)\n";
var_dump(filter_var_array(NULL));
var_dump(filter_var_array(array()));
var_dump(filter_var_array(array(1,"blah"=>"hoho")));
var_dump(filter_var_array(array(), -1));
var_dump(filter_var_array(array(), 1000000));
var_dump(filter_var_array(array(), ""));

echo "-- (2)\n";
var_dump(filter_var_array(darray[""=>""], -1));
var_dump(filter_var_array(darray[""=>""], 1000000));
var_dump(filter_var_array(darray[""=>""], ""));

echo "-- (3)\n";
var_dump(filter_var_array(darray["aaa"=>"bbb"], -1));
var_dump(filter_var_array(darray["aaa"=>"bbb"], 1000000));
var_dump(filter_var_array(darray["aaa"=>"bbb"], ""));

echo "-- (4)\n";
var_dump(filter_var_array(array(), new stdclass));
var_dump(filter_var_array(array(), array()));
var_dump(filter_var_array(array(), darray["var_name"=>1]));
var_dump(filter_var_array(array(), darray["var_name"=>-1]));
var_dump(filter_var_array(darray["var_name"=>""], darray["var_name"=>-1]));

echo "-- (5)\n";
var_dump(filter_var_array(darray["var_name"=>""], darray["var_name"=>-1, "asdas"=>"asdasd", "qwe"=>"rty", ""=>""]));
var_dump(filter_var_array(darray["asdas"=>"text"], darray["var_name"=>-1, "asdas"=>"asdasd", "qwe"=>"rty", ""=>""]));


$a = darray[""=>""]; $b = -1;
var_dump(filter_var_array($a, $b));
var_dump($a, $b);

$a = darray[""=>""]; $b = 100000;
var_dump(filter_var_array($a, $b));
var_dump($a, $b);

$a = darray[""=>""]; $b = "";
var_dump(filter_var_array($a, $b));
var_dump($a, $b);

echo "Done\n";
}
