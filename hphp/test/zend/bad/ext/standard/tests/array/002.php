<?php
require(dirname(__FILE__) . '/data.inc');

function test_sort ($sort_function, $data) {
    echo "\n -- Testing $sort_function() -- \n";
	echo "No second argument:\n";
    $sort_function ($data);
    var_dump ($data);
	echo "Using SORT_REGULAR:\n";
    $sort_function ($data, SORT_REGULAR);
    var_dump ($data);
	echo "Using SORT_NUMERIC:\n";
    $sort_function ($data, SORT_NUMERIC);
    var_dump ($data);
	echo "Using SORT_STRING\n";
    $sort_function ($data, SORT_STRING);
    var_dump ($data);
}

echo "Unsorted data:\n";
var_dump ($data);
foreach (array ('arsort', 'asort', 'krsort', 'ksort', 'rsort', 'sort') as $test_function) {
    test_sort ($test_function, $data);
}

?>