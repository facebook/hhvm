<?php
	include "resourcebundle.inc";

	$r = new ResourceBundle( 'es', BUNDLE );

	var_dump($r instanceof Traversable);
	var_dump(iterator_to_array($r->get('testarray')));
?>