<?php
$date = new DateTime( '2009-07-29 16:44:23 Europe/London' );
$date->modify( "+1.61538461538 day" );
echo $date->format( 'r' ), "\n";

$date = new DateTime( '2009-07-29 16:44:23 Europe/London' );
$date->modify( "61538461538 day" );
echo $date->format( 'r' ), "\n";

$date = new DateTime( '2009-07-29 16:44:23 Europe/London' );
$date->modify( "£61538461538 day" );
echo $date->format( 'r' ), "\n";
?>