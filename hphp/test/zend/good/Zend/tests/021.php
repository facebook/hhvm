<?hh <<__EntryPoint>> function main(): void {
var_dump(true ?: false);
var_dump(false ?: true);
var_dump(23 ?: 42);
var_dump(0 ?: "bar");

$a = 23;
$b = 0;
$c = "";
$d = 23.5;

var_dump($a ?: $b);
var_dump($c ?: $d);

var_dump(1 ?: print(2));

$e = dict[];

$e['e'] = 'e';
$e['e'] = $e['e'] ?: 'e';
print_r($e);
}
