<?hh <<__EntryPoint>> function main(): void {
$a = vec[1,2,3];
$b = dict['a'=>1,'b'=>1,'c'=>2];

var_dump(
        array_slice($a, 1),
        array_slice($a, 1, 2, TRUE),
        array_slice($a, 1, NULL, TRUE),
        array_slice($b, 1),
        array_slice($b, 1, 2, TRUE),
        array_slice($b, 1, NULL, TRUE)
);

echo "Done\n";
}
