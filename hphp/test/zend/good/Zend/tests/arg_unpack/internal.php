<?hh
<<__EntryPoint>> function main(): void {
$arrays = varray[
    varray[1, 2, 3],
    varray[4, 5, 6],
    varray[7, 8, 9],
];
var_dump(array_map(null, ...$arrays));
}
