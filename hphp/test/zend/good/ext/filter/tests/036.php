<?hh
<<__EntryPoint>> function main(): void {
$var = "1";

$data = dict['test1' => 1, 'test2' => $var];

$args = dict[];
$args["test1"] = FILTER_VALIDATE_INT;
$args["test2"] = FILTER_VALIDATE_INT;

$ret = filter_var_array($data, $args);
var_dump($ret);
var_dump($data); //should be separated, i.e. not reference anymore. looks like we can't change this, or it'd change the original zval instead..
var_dump($var); //should be still string(1) "1"

echo "Done\n";
}
