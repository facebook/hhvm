<?hh <<__EntryPoint>> function main(): void {
$data = darray[
    'product_id'    => 'libgd<script>',
    'component'     => '10dhsajkkdhk <do>',
    'versions'      => '2.0.33',
    'testscalar'    => varray['2','23','10','12'],
    'testarray'     => '2',
];

$args = darray[
    'product_id'    => FILTER_SANITIZE_ENCODED,
    'component'     => darray[//'filter' => FILTER_VALIDATE_INT,
                             'flags'    => FILTER_FORCE_ARRAY,
                             'options'  => darray["min_range"=>1, "max_range"=>10]
                        ],
    'versions'      => darray[
                            'filter' => FILTER_SANITIZE_ENCODED,
                            'flags'  => FILTER_REQUIRE_SCALAR,
                            ],
    'doesnotexist'  => FILTER_VALIDATE_INT,
    'testscalar'    => FILTER_VALIDATE_INT,
    'testarray' => darray[
                            'filter' => FILTER_VALIDATE_INT,
                            'flags'  => FILTER_FORCE_ARRAY,
                        ]

];

$myinputs = filter_var_array($data, $args);
var_dump($myinputs);
}
