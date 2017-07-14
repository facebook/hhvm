<?php
function a ($option) {
	b($option['bla']);
	c($option);
	var_dump($option);
}
function b (&$string) {
	$string = 'changed';
}
function c ($option) {
	switch ($option['bla']) {
	default:
		$copy = $option;
		$copy['bla'] = 'copy';
		break;
	}
}
a(array('bla' => 'fasel'));

?>
