<?hh <<__EntryPoint>> function main(): void {
$data = dict[
    'product_id'    => 'libgd<script>',
    'component'     => '10dhsajkkdhk <do>',
    'versions'      => '2.0.33',
    'testscalar'    => vec['2','23','10','12'],
    'testarray'     => '2',
];

$args = dict[
    'product_id'    => FILTER_SANITIZE_ENCODED,
    'component'     => dict['flags'    => FILTER_FORCE_ARRAY,
                             'options'  => dict["min_range"=>1, "max_range"=>10]
                        ],
    'versions'      => dict[
                            'filter' => FILTER_SANITIZE_ENCODED,
                            'flags'  => FILTER_REQUIRE_SCALAR,
                            ],
    'doesnotexist'  => FILTER_VALIDATE_INT,
    'testscalar'    => FILTER_VALIDATE_INT,
    'testarray' => dict[
                            'filter' => FILTER_VALIDATE_INT,
                            'flags'  => FILTER_FORCE_ARRAY,
                        ]

];
$out = filter_var_array($data, $args);
var_dump($out);
}
