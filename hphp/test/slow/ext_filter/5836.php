<?hh

<<__EntryPoint>>
function main_5836() :mixed{
$data = darray[
  'true'     => '1',
  'false'    => '0',
  'badbool'  => 'xyzzy',
  'bademail' => 'foo',
  'badfloat' => 'idkfa',
  'badint'   => '42.42',
  'badip'    => '256.0.0.1',
];

$args = darray[
  'true'    => FILTER_VALIDATE_BOOLEAN,
  'false'   => FILTER_VALIDATE_BOOLEAN,
  'badbool' => darray[
    'filter' => FILTER_VALIDATE_BOOLEAN,
    'flags'  => FILTER_NULL_ON_FAILURE,
  ],
  'bademail' => FILTER_VALIDATE_EMAIL,
  'badfloat' => FILTER_VALIDATE_FLOAT,
  'badint'   => FILTER_VALIDATE_INT,
  'badip'    => FILTER_VALIDATE_IP,
];

var_dump(filter_var_array($data, $args));
}
