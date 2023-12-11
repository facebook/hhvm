<?hh

<<__EntryPoint>> function main(): void {
echo "*** Test substituting argument 1 with array values ***\n";



$index_array = vec[1, 2, 3];
$assoc_array = dict[1 => 'one', 2 => 'two'];

$variation_array = dict[
  'empty array' => vec[],
  'int indexed array' => $index_array,
  'associative array' => $assoc_array,
  'nested arrays' => vec['foo', $index_array, $assoc_array],
  ];


foreach ( $variation_array as $var ) {
  var_dump(posix_ttyname( $var  ) );
}
}
