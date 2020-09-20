<?hh

<<__EntryPoint>> function main(): void {
echo "*** Test substituting argument 1 with boolean values ***\n";



$variation_array = darray[
  'lowercase true' => true,
  'lowercase false' =>false,
  'uppercase TRUE' =>TRUE,
  'uppercase FALSE' =>FALSE,
  ];


foreach ( $variation_array as $var ) {
  var_dump(posix_ttyname( $var  ) );
}
}
