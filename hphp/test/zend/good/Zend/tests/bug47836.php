<?hh <<__EntryPoint>> function main(): void {
$arr = varray[];
$arr[PHP_INT_MAX] = 1;
$arr[] = 2;

var_dump($arr);
}
