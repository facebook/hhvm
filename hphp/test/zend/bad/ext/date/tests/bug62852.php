<?php
$s1 = 'O:8:"DateTime":3:{s:4:"date";s:20:"10007-06-07 03:51:49";s:13:"timezone_type";i:3;s:8:"timezone";s:3:"UTC";}';

try {
    unserialize( $s1 );
} catch ( Exception $e ) {}
