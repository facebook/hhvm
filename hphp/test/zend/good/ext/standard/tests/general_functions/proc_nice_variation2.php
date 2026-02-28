<?hh

<<__EntryPoint>> function main(): void {
echo "*** Test substituting argument 1 with boolean values ***\n";



$variation_array = dict[
  'lowercase true' => true,
  'lowercase false' =>false,
  'uppercase TRUE' =>TRUE,
  'uppercase FALSE' =>FALSE,
  ];


foreach ( $variation_array as $var ) {
  var_dump(proc_nice((int)$var  ) );
}
}
