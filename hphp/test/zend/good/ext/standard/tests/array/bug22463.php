<?hh

function a($ary) :mixed{
    return (is_array($ary) ? array_reduce($ary, cb<>, 0) : 1);
}

function cb($v, $elem) :mixed{
    return $v + a($elem);
}
<<__EntryPoint>> function main(): void {
$ary = vec[
    vec[
        vec[
            vec[
                vec[
                    vec[0, 1, 2, 3, 4]
                ]
            ]
        ]
    ]
];

var_dump(a($ary));
}
