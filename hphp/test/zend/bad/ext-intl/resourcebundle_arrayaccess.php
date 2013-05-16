<?php
	include "resourcebundle.inc";

	// fall back
	$r = new ResourceBundle( 'en_US', BUNDLE );

	printf( "length: %d\n", count($r) );
	printf( "teststring: %s\n", $r['teststring'] );
	printf( "testint: %d\n", $r['testint'] );

	print_r( $r['testvector'] );

	printf( "testbin: %s\n", bin2hex($r['testbin']) );

	$r2 = $r['testtable'];
	printf( "testtable: %d\n", $r2['major'] );

	$r2 = $r['testarray'];
	printf( "testarray: %s\n", $r2[2] );

	$t = $r['nonexisting'];
	echo debug( $t );
?>