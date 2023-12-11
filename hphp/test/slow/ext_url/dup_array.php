<?hh


<<__EntryPoint>>
function main_dup_array() :mixed{
$nested = dict[
  'key1' => dict[
    'subkey1' => 'subval1',
    'subkey2' => 'subval2'
  ],
  'key2' => dict[
    'subkey1' => 'subval1',
    'subkey2' => 'subval2'
  ],
];

echo http_build_query($nested), "\n";

$subarr = Map {'subkey1'=>'subval1', 'subkey2'=>'subval2'};
$nested = dict['key1'=>$subarr, 'key2'=>$subarr];

echo http_build_query($nested), "\n";
}
