<?hh

$nested = array(
  'key1' => array(
    'subkey1' => 'subval1',
    'subkey2' => 'subval2'
  ),
  'key2' => array(
    'subkey1' => 'subval1',
    'subkey2' => 'subval2'
  ),
);

echo http_build_query($nested), "\n";

$subarr = Map {'subkey1'=>'subval1', 'subkey2'=>'subval2'};
$nested = array('key1'=>$subarr, 'key2'=>$subarr);

echo http_build_query($nested), "\n";
