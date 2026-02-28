<?hh <<__EntryPoint>> function main(): void {
$a = dict[
    'a1' => 1,
    'a2' => vec[ 1, 2, 3 ],
    'a3' => dict[
        'a' => vec[ 10, 20, 30 ],
        'b' => 'b'
        ]
    ];
$b = dict[ 'a1' => 2,
    'a2' => vec[ 3, 4, 5 ],
    'a3' => dict[
        'c' => 'cc',
        'a' => vec[ 10, 40 ]
        ]
    ];

var_dump($a);
array_merge_recursive( $a, $b );
var_dump($a);
}
