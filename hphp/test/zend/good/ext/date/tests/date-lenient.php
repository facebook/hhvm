<?php
$date = "06/08/04 12:00";
print_r( date_parse_from_format( 'm/d/y', $date ) );
print_r( date_parse_from_format( 'm/d/y+', $date ) );
print_r( date_parse_from_format( '+m/d/y', $date ) );
print_r( date_parse_from_format( 'm/d/y++', $date ) );

$date = "06/08/04";
print_r( date_parse_from_format( 'm/d/y+', $date ) );
print_r( date_parse_from_format( '+m/d/y', $date ) );

?>