<?hh <<__EntryPoint>> function main(): void {

$variation_array = dict[
  'empty string DQ' => "",
  'empty string SQ' => '',
  'uppercase NULL' => NULL,
  'lowercase null' => null,
  ];


foreach ( $variation_array as $var ) {
  try { var_dump(fstat( $var  ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
echo "===DONE===\n";
}
