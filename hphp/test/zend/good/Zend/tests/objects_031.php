<?hh <<__EntryPoint>> function main(): void {
$x = varray[];
$x[] = clone new stdClass;
$x[] = clone new stdClass;
$x[] = clone new stdClass;

$x[0]->a = 1;

var_dump($x);
}
