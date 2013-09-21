<?php
echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

$extra_args = array( 12312, 2 => '1234', 'string' => 'string' );

var_dump( posix_geteuid( $extra_args ));
foreach ( $extra_args as $arg )
{
	var_dump(posix_geteuid( $arg ));
}

?>