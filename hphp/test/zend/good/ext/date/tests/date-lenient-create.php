<?php
$tz = new DateTimeZone("UTC");
$date = "06/08/04 12:00";
echo "==\n";
print_r( date_create_from_format( 'm/d/y', $date , $tz) );
print_r( date_get_last_errors() );
echo "==\n";
print_r( date_create_from_format( 'm/d/y+', $date , $tz)->setTime(0, 0) );
print_r( date_get_last_errors() );
echo "==\n";
print_r( date_create_from_format( '+m/d/y', $date , $tz)->setTime(0, 0) );
print_r( date_get_last_errors() );
echo "==\n";
print_r( date_create_from_format( 'm/d/y++', $date , $tz)->setTime(0, 0) );
print_r( date_get_last_errors() );
echo "==\n";

$date = "06/08/04";
print_r( date_create_from_format( 'm/d/y+', $date , $tz)->setTime(0, 0) );
print_r( date_get_last_errors() );
echo "==\n";
print_r( date_create_from_format( '+m/d/y', $date , $tz)->setTime(0, 0) );
print_r( date_get_last_errors() );
echo "==\n";

?>