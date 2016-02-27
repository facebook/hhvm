<?php
$current = "2014-03-02 16:24:08";

$i = DateTimeImmutable::createFromMutable( date_create( $current ) );
var_dump( $i );

$i = DateTimeImmutable::createFromMutable( date_create_immutable( $current ) );
var_dump( $i );
?>
