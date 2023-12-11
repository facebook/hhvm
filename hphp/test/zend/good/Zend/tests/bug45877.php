<?hh <<__EntryPoint>> function main(): void {
$keys = vec[PHP_INT_MAX,
    (string) PHP_INT_MAX,
    (string) (-PHP_INT_MAX - 1),
    -PHP_INT_MAX - 1,
    (string) (PHP_INT_MAX + 1)];

var_dump(array_fill_keys($keys, 1));
}
