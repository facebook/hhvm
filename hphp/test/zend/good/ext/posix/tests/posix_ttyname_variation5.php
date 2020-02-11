<?hh

<<__EntryPoint>> function main(): void {
echo "*** Test substituting argument 1 with int values ***\n";



$variation_array = darray [
    'int 12345' => 12345,
    'int -12345' => -2345,
    ];


foreach ( $variation_array as $var ) {
  var_dump(posix_ttyname( $var  ) );
}
}
