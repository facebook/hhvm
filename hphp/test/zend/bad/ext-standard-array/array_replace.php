<?php

$array1 = array(
	0 => 'dontclobber',
	'1' => 'unclobbered',
	'test2' => 0.0,
	'test3' => array(
		'testarray2' => true,
		1 => array(
			'testsubarray1' => 'dontclobber2',
			'testsubarray2' => 'dontclobber3',
	),
    ),
);

$array2 = array(
	1 => 'clobbered',
	'test3' => array(
		'testarray2' => false,
	),
	'test4' => array(
		'clobbered3' => array(0, 1, 2),
	),
);

$array3 = array(array(array(array())));

$array4 = array();
$array4[] = &$array4;

echo " -- Testing array_replace() --\n";
$data = array_replace($array1, $array2);

var_dump($data);

echo " -- Testing array_replace_recursive() --\n";
$data = array_replace_recursive($array1, $array2);

var_dump($data);

echo " -- Testing array_replace_recursive() w/ endless recusrsion --\n";
$data = array_replace_recursive($array3, $array4);

var_dump($data);
?>