<?hh

<<__EntryPoint>> function main(): void {
include_once( 'ut_common.inc' );
\HH\global_set('oo-mode', true);
$res_str = '';
/*
 * Clone
 */
$fmt = ut_msgfmt_create( "en_US", "{0,number} monkeys on {1,number} trees" );

// Get default patten.
$res_str .= "Formatting result: " . ut_msgfmt_format( $fmt, dict[0 => 123, 1 => 456] ) . "\n";
$fmt_clone = clone $fmt;
// Set a new pattern.
$pattern = "{0,number} trees hosting {1,number} monkeys";
$res = ut_msgfmt_set_pattern( $fmt, $pattern );
$res_str .= "Formatting result: " . ut_msgfmt_format( $fmt, dict[0 => 123, 1 => 456] ) . "\n";
$res_str .= "Formatting clone result: " . ut_msgfmt_format( $fmt_clone, dict[0 => 123, 1 => 456] ) . "\n";

echo $res_str;
}
