<?php
$arr = array(
	'a' => array(
		'a' => 'apple',
		'b' => 'banana',
		'c' => 'cranberry',
		'd' => 'mango',
		'e' => 'pineapple'
	),
	'b' => array(
		'a' => 'apple',
		'b' => 'banana',
		'c' => 'cranberry',
		'd' => 'mango',
		'e' => 'pineapple'
	),
	'c' => 'cranberry',
	'd' => 'mango',
	'e' => 'pineapple'
);

function test(&$child, $entry)
{
	$i = 1;

	foreach ($child AS $key => $fruit)
	{
		if (!is_numeric($key))
		{
			$child[$i] = $fruit;
			unset($child[$key]);
			$i++;
		}
	}
}

$i = 1;

foreach ($arr AS $key => $fruit)
{
	$arr[$i] = $fruit;

	if (is_array($fruit))
	{
		test($arr[$i], $fruit);
	}

	unset($arr[$key]);
	$i++;
}

var_dump($arr);
?>
