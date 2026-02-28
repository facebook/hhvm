<?hh
/*
 * proto array array_splice(array input, int offset [, int length [, array replacement]])
 * Function is implemented in ext/standard/array.c
*/

function test_splice ($replacement)
:mixed{
    $input_array=vec[0,1];
    var_dump (array_splice(inout $input_array, 2,0,$replacement));
    var_dump ($input_array);
}
<<__EntryPoint>> function main(): void {
test_splice (2);

test_splice (2.1);

test_splice (true);
//file type resource
$file_handle = fopen(__FILE__, "r");

test_splice ($file_handle);
echo "Done\n";
}
