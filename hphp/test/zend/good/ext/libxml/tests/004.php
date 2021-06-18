<?hh
<<__EntryPoint>> function main(): void {
$ctxs = varray[
	NULL,
	'bogus',
	123,
	new stdClass,
	varray['a'],
	stream_context_create(),
	stream_context_create(darray[0 => 'file']),
	stream_context_create(darray['file' => darray['some_opt' => 'aaa']])
];


foreach ($ctxs as $ctx) {
	try { var_dump(libxml_set_streams_context($ctx)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
	$dom = new DOMDocument();
	var_dump($dom->load(dirname(__FILE__).'/test.xml'));
}

echo "Done\n";
}
