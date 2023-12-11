<?hh <<__EntryPoint>> function main(): void {
$variation_array = dict[
  'lowercase true' => true,
  'lowercase false' =>false,
  'uppercase TRUE' =>TRUE,
  'uppercase FALSE' =>FALSE,
  ];


foreach ( $variation_array as $var ) {
  try { var_dump(fstat( $var  ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
echo "===DONE===\n";
}
