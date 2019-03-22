<?php

include "resourcebundle.inc";

function ut_main($bundle) {
	$str_res = '';

	$str_res .= join("\n", ut_resourcebundle_locales($bundle));

	return $str_res;
}

	include_once( 'ut_common.inc' );
function ut_run2($bundle)
	{
	    // Run unit test in OO mode.
	    $GLOBALS['oo-mode'] = true;
	    $oo_result = ut_main($bundle);

	    // Run unit test in procedural mode.
	    $GLOBALS['oo-mode'] = false;
	    $proc_result = ut_main($bundle);

	    // Show error if the APIs produce different results.
	    if( $proc_result !== $oo_result )
	    {
	          echo "ERROR: OO- and procedural APIs produce different results!\n";
	          echo "OO API output:\n";
	          echo str_repeat( '=', 78 ) . "\n";
	          echo $oo_result;
	          echo str_repeat( '=', 78 ) . "\n";
	          echo "procedural API output:\n";
	          echo str_repeat( '=', 78 ) . "\n";
	          echo $proc_result;
	          echo str_repeat( '=', 78 ) . "\n";
	          return;
	    }

	    // Else, if the results are equal, show one of them.
	    echo $proc_result;
	}
	ut_run2($bundle);
