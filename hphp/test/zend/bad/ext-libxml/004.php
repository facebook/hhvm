<?php

$ctxs = array(
	NULL,
	'bogus',
	123,
	new stdclass,
	array('a'),
	stream_context_create(),
	stream_context_create(array('file')),
	stream_context_create(array('file' => array('some_opt' => 'aaa')))
);


foreach ($ctxs as $ctx) {
	var_dump(libxml_set_streams_context($ctx));
	$dom = new DOMDocument();
	var_dump($dom->load(dirname(__FILE__).'/test.xml'));
}

echo "Done\n";

?>