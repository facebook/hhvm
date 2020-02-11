<?hh <<__EntryPoint>> function main(): void {
$a = darray[
    'a1' => 1,
    'a2' => varray[ 1, 2, 3 ],
    'a3' => darray[
        'a' => varray[ 10, 20, 30 ],
        'b' => 'b'
        ]
    ];
$b = darray[ 'a1' => 2,
    'a2' => varray[ 3, 4, 5 ],
    'a3' => darray[
        'c' => 'cc',
        'a' => varray[ 10, 40 ]
        ]
    ];

var_dump($a);
array_merge_recursive( $a, $b );
var_dump($a);
}
