<?hh
<<__EntryPoint>> function main(): void {
$array1 = darray[
       'friends' => 5,
       'children' => darray[
               'dogs' => 0,
       ],
];

$array2 = darray[
       'friends' => 10,
       'children' => darray[
               'cats' => 5,
       ],
];

$merged = array_merge_recursive($array1, $array2);

var_dump($array1, $array2);
}
