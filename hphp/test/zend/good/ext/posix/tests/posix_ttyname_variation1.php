<?hh

<<__EntryPoint>> function main(): void {
echo "*** Test substituting argument 1 with array values ***\n";



$index_array = varray[1, 2, 3];
$assoc_array = darray[1 => 'one', 2 => 'two'];

$variation_array = darray[
  'empty array' => varray[],
  'int indexed array' => $index_array,
  'associative array' => $assoc_array,
  'nested arrays' => varray['foo', $index_array, $assoc_array],
  ];


foreach ( $variation_array as $var ) {
  var_dump(posix_ttyname( $var  ) );
}
}
