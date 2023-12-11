<?hh

<<__EntryPoint>> function main(): void {
$use_include_path = false;


$index_array = vec[1, 2, 3];
$assoc_array = dict[1 => 'one', 2 => 'two'];

$variation = dict[
  'empty array' => vec[],
  'int indexed array' => $index_array,
  'associative array' => $assoc_array,
  'nested arrays' => vec['foo', $index_array, $assoc_array],
  ];


foreach ( $variation as $var ) {
  try { var_dump(gzfile( $var ,  $use_include_path ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
echo "===DONE===\n";
}
