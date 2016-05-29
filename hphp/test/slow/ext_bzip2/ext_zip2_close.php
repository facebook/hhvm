<?php

ini_set( 'display_errors', 'stderr' );
$reader = new XMLReader();
$url = "compress.bzip2://".dirname(__FILE__)."/book.xml.bz2";
$reader->open( $url, null );
$reader->read();
