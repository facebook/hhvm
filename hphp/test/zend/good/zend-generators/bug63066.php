<?php
function gen($o)
{
	yield 'foo';
	$o->fatalError();
}

foreach(gen(new stdClass()) as $value)
	echo $value, "\n";