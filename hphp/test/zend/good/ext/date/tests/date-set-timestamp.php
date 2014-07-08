<?php
date_default_timezone_set('Europe/Oslo');
$d = new DateTime( '@1217184864' );
echo $d->format( "Y-m-d H:i e\n" );

$d = new DateTime();
$d->setTimestamp( 1217184864 );
echo $d->format( "Y-m-d H:i e\n" );
?>