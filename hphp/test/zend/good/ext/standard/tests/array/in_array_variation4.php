<?hh
/*
 * Prototype  : bool in_array ( mixed $needle, array $haystack [, bool $strict] )
 * Description: Searches haystack for needle and returns TRUE
 *              if it is found in the array, FALSE otherwise.
 * Source Code: ext/standard/array.c
*/

/* Test in_array() with haystack as resouce and multidimentional arrays */

/* checking for Resources */
<<__EntryPoint>> function main(): void {
echo "*** Testing resource type with in_array() ***\n";
//file type resource
$file_handle = fopen(__FILE__, "r");

//directory type resource
$dir_handle = opendir( dirname(__FILE__) );

//store resources in array for comparison.
$resources = vec[$file_handle, $dir_handle];

// search for resouce type in the resource array
var_dump( in_array($file_handle, $resources, true) );
//checking for (int) type resource
var_dump( in_array((int)$dir_handle, $resources, true) );

/* Miscellenous input check  */
echo "\n*** Testing miscelleneos inputs with in_array() ***\n";
//matching "Good" in array(0,"hello"), result:true in loose type check
var_dump( in_array("Good", vec[0,"hello"]) );
//false in strict mode
var_dump( in_array("Good", vec[0,"hello"], TRUE) );

//matching integer 0 in array("this"), result:true in loose type check
var_dump( in_array(0, vec["this"]) );
// false in strict mode
var_dump( in_array(0, vec["this"]),TRUE );

//matching string "this" in array(0), result:true in loose type check
var_dump( in_array("this", vec[0]) );
// false in stric mode
var_dump( in_array("this", vec[0], TRUE) );

//checking for type FALSE in multidimensional array with loose checking, result:false in loose type check
var_dump( in_array(FALSE,
                   dict["a"=> TRUE, "b"=> TRUE,
                          0 => dict["c"=> TRUE, "d"=>TRUE]
                         ]
                  )
        );

//matching string having integer in beginning, result:true in loose type check
var_dump( in_array('123abc', vec[123]) );
var_dump( in_array('123abc', vec[123], TRUE) ); // false in strict mode

echo "Done\n";
}
