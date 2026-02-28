<?hh
<<__EntryPoint>> function main(): void {
$array1 = dict[
       'friends' => 5,
       'children' => dict[
               'dogs' => 0,
       ],
];

$array2 = dict[
       'friends' => 10,
       'children' => dict[
               'cats' => 5,
       ],
];

$merged = array_merge_recursive($array1, $array2);

var_dump($array1, $array2);
}
