<?php
	include "resourcebundle.inc";

function ut_main() {
	$str_res = '';
	// fall back
	$r = ut_resourcebundle_create( 'en_US', BUNDLE );

	$str_res .= sprintf( "length: %d\n", ut_resourcebundle_count($r) );
	$str_res .= sprintf( "teststring: %s\n", ut_resourcebundle_get($r,  'teststring' ) );
	$str_res .= sprintf( "testint: %d\n", ut_resourcebundle_get($r, 'testint' ) );

	$str_res .= print_r( ut_resourcebundle_get($r, 'testvector' ), true );

	$str_res .= sprintf( "testbin: %s\n", bin2hex(ut_resourcebundle_get( $r,'testbin' )) );

	$r2 = ut_resourcebundle_get($r, 'testtable' );
	$str_res .= sprintf( "testtable: %d\n", ut_resourcebundle_get($r2, 'major' ) );

	$r2 = ut_resourcebundle_get($r,'testarray' );
	$str_res .= sprintf( "testarray: %s\n", ut_resourcebundle_get($r2, 2 ) );

	$t = ut_resourcebundle_get( $r, 'nonexisting' );
	$str_res .= debug( $t );
	
	return $str_res;
}
	include_once( 'ut_common.inc' );
	ut_run();
?>