<?hh

<<__EntryPoint>> function main(): void {
$variation = dict[
  'lowercase true' => true,
  'lowercase false' =>false,
  'uppercase TRUE' =>TRUE,
  'uppercase FALSE' =>FALSE,
  ];


foreach ( $variation as $var ) {
  try { var_dump(readgzfile( '' ,  $var ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
echo "===DONE===\n";
}
