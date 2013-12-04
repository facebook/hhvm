<?php
	mb_internal_encoding( 'ISO-8859-1' );
	print mb_strtolower( "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n" );
	print mb_strtoupper( mb_strtolower( "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n" ) );
	print mb_strtoupper( "הכן\n" );
	print mb_convert_case( "הכן\n", MB_CASE_TITLE );
?>
