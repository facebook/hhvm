<?php
function setUp()
{
    $handler = new PDO( "sqlite::memory:" );
    $handler->sqliteCreateFunction( "md5", "md5", 1 );
    unset( $handler );
}

setUp();
setUp();
echo "done";
?>