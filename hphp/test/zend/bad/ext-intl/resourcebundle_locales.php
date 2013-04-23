<?php
	
include "resourcebundle.inc";

function ut_main() {
	$str_res = '';

	$str_res .= join("\n", ut_resourcebundle_locales(BUNDLE));
	
	return $str_res;
}

	include_once( 'ut_common.inc' );
	ut_run();
?>