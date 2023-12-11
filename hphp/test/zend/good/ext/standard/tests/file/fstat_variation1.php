<?hh <<__EntryPoint>> function main(): void {
$index_array = vec[1, 2, 3];
$assoc_array = dict[1 => 'one', 2 => 'two'];

$variation_array = dict[
  'empty array' => vec[],
  'int indexed array' => $index_array,
  'associative array' => $assoc_array,
  'nested arrays' => vec['foo', $index_array, $assoc_array],
  ];


foreach ( $variation_array as $var ) {
  try { var_dump(fstat( $var  ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
echo "===DONE===\n";
}
