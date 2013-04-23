<?php
	include "resourcebundle.inc";

	// fall back
	$r = new ResourceBundle( 'en_US', BUNDLE );

	foreach ($r as $onekey => $oneval) {
		echo "Here comes $onekey:\n";
		switch (gettype($oneval)) {
		  case 'string':
		    echo bin2hex( $oneval ) . "\n";
		    break;

		  case 'integer':
		    echo "$oneval\n";
		    break;

		  default:
		    print_r( $oneval );
		}
		echo "\n";
	}

	echo "Testarray Contents:\n";
	$r = $r->get( 'testarray' );
	foreach ($r as $onekey => $oneval) {
	   echo "$onekey => $oneval\n";
	}
?>