<?php
	mb_regex_set_options( '' );
	print mb_ereg_replace( ' ', '-', 'a b c d e' )."\n";
	print mb_ereg_replace( '([a-z]+)','[\\1]', 'abc def ghi' );
?>

