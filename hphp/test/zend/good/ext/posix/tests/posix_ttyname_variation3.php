<?hh

<<__EntryPoint>> function main(): void {
echo "*** Test substituting argument 1 with emptyUnsetUndefNull values ***\n";




$variation_array = dict[
  'empty string DQ' => "",
  'empty string SQ' => '',
  'uppercase NULL' => NULL,
  'lowercase null' => null,
  ];


foreach ( $variation_array as $var ) {
  var_dump(posix_ttyname( $var  ) );
}
}
