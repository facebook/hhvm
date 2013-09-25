<?php

function callback($match)
{
	var_dump($match);
	return $match[1].'/'.strlen($match['name']);
}

var_dump(preg_replace_callback('|(?P<name>blub)|', 'callback', 'bla blub blah'));

var_dump(preg_match('|(?P<name>blub)|', 'bla blub blah', $m));
var_dump($m);

var_dump(preg_replace_callback('|(?P<1>blub)|', 'callback', 'bla blub blah'));

?>