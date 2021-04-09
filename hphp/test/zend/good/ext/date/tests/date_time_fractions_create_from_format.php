<?hh

<<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');

$dt = date_create_from_format( "Y-m-d H:i:s.u", "2016-10-03 12:47:18.819313" );
var_dump( $dt );

$dt = date_create_from_format( "U.u", "1475500799.176312" );
var_dump( $dt );
}
