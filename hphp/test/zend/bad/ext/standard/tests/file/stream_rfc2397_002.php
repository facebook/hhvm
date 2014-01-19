<?php
ini_set('allow_url_fopen', 1);


$streams = array(
	'data://,',
	'data://',
	'data://;base64,',
	'data://;base64',
	'data://foo,',
	'data://foo=bar,',
	'data://text/plain,',
	'data://text/plain;foo,',
	'data://text/plain;foo=bar,',
	'data://text/plain;foo=bar;bla,',
	'data://text/plain;foo=bar;base64,',
	'data://text/plain;foo=bar;bar=baz',
	'data://text/plain;foo=bar;bar=baz,',
	);

foreach($streams as $stream)
{
	$stream = fopen($stream, 'r');
	$meta = @stream_get_meta_data($stream);
	var_dump($meta);
	var_dump(isset($meta['foo']) ? $meta['foo'] : null);
}

?>
===DONE===
<?php
ini_set('allow_url_fopen', 1);
 exit(0); ?>