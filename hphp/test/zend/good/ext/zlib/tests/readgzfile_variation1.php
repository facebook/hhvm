<?hh

<<__EntryPoint>> function main(): void {
$use_include_path = false;


$index_array = varray[1, 2, 3];
$assoc_array = darray[1 => 'one', 2 => 'two'];

$variation = darray[
  'empty array' => varray[],
  'int indexed array' => $index_array,
  'associative array' => $assoc_array,
  'nested arrays' => varray['foo', $index_array, $assoc_array],
  ];


foreach ( $variation as $var ) {
  try { var_dump(readgzfile( $var ,  $use_include_path ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
echo "===DONE===\n";
}
