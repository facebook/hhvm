<?php
/* 
 * proto array array_splice(array input, int offset [, int length [, array replacement]])
 * Function is implemented in ext/standard/array.c
*/ 

echo "*** array_splice() function : usage variations - lengths and offsets\n";


function test_splice ($offset, $length)
{
	echo "  - No replacement\n";
	$input_array=array(0,1,2,3,4,5);
	var_dump (array_splice ($input_array,$offset,$length));
	var_dump ($input_array);
    echo "  - With replacement\n";
    $input_array=array(0,1,2,3,4,5);
    var_dump (array_splice ($input_array,$offset,$length,array ("A","B","C")));
	var_dump ($input_array);
}

echo "absolute offset - absolute length - cut from beginning\n";
test_splice (0,2);
echo "absolute offset - absolute length - cut from middle\n";
test_splice (2,2);
echo "absolute offset - absolute length - cut from end\n";
test_splice (4,2);
echo "absolute offset - absolute length - attempt to cut past end\n";
test_splice (4,4);
echo "absolute offset - absolute length - cut everything\n";
test_splice (0,7);
echo "absolute offset - absolute length - cut nothing\n";
test_splice (3,0);

echo "absolute offset - relative length - cut from beginning\n";
test_splice (0,-4);

echo "absolute offset - relative length - cut from middle\n";
test_splice (2,-2);

echo "absolute offset - relative length - attempt to cut form before beginning \n";
test_splice (0,-7);

echo "absolute offset - relative length - cut nothing\n";
test_splice (2,-7);

echo "relative offset - absolute length - cut from beginning\n";
test_splice (-6,2);

echo "relative offset - absolute length - cut from middle\n";
test_splice (-4,2);
echo "relative offset - absolute length - cut from end\n";
test_splice (-2,2);
echo "relative offset - absolute length - attempt to cut past end\n";
test_splice (-2,4);
echo "relative offset - absolute length - cut everything\n";
test_splice (-6,6);
echo "relative offset - absolute length - cut nothing\n";
test_splice (-6,0);

echo "relative offset - relative length - cut from beginning\n";
test_splice (-6,-4);

echo "relative offset - relative length - cut from middle\n";
test_splice (-4,-2);

echo "relative offset - relative length - cut nothing\n";
test_splice (-4,-7);
echo "Done\n";
?>
