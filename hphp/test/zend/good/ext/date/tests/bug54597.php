<?php
$tz = new DateTimeZone("Europe/Amsterdam");
$dateObject = new DateTime( 'January 0099', $tz );
echo $dateObject->format( 'Y' ), "\n";
$dateObject = new DateTime( 'January 1, 0099', $tz );
echo $dateObject->format( 'Y' ), "\n";
$dateObject = new DateTime( '0099-01', $tz );
echo $dateObject->format( 'Y' ), "\n";
?>