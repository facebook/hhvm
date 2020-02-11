<?hh <<__EntryPoint>> function main(): void {
$a = darray[
    'a' => varray[
        'A', 'B', 'C', 'D',
    ],
    'b' => varray[
        'AA', 'BB', 'CC', 'DD',
    ],
];

// Set the pointer of $a to 'b' and the pointer of 'b' to 'CC'
reset(inout $a);
next(inout $a);
$ab = $a['b'];
next(inout $ab);
next(inout $ab);
next(inout $ab);

var_dump(key($ab));
foreach($a as $k => $d)
{
}
// Alternatively $c = $a; and foreachloop removal will cause identical results.
var_dump(key($ab));
}
