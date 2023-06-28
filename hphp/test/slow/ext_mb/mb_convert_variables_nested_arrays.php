<?hh


<<__EntryPoint>>
function main_mb_convert_variables_nested_arrays() :mixed{
$a = darray[
  'test' => darray[
    'sub_test' => varray[

    ],
  ],
];

mb_convert_variables( 'utf-8', 'windows-1251', inout $a);
var_dump($a);
}
