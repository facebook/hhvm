<?php

function replace_variables($text, $params) {
	
	$c = function($matches) use (&$params, &$text) {
		$text = preg_replace( '/(\?)/', array_shift( $params ), $text, 1 );
	};

	preg_replace_callback( '/(\?)/', $c, $text );
	
	return $text;
}

echo replace_variables('a=?', array('0')) . "\n";
echo replace_variables('a=?, b=?', array('0', '1')) . "\n";
echo replace_variables('a=?, b=?, c=?', array('0', '1', '2')) . "\n";
echo "Done\n";
?>
