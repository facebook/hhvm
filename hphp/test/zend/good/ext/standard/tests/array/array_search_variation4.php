<?hh
/*
 * Prototype  : mixed array_search ( mixed $needle, array $haystack [, bool $strict] )
 * Description: Searches haystack for needle and returns the key if it is found in the array, FALSE otherwise
 * Source Code: ext/standard/array.c
*/

/* checking for Resources */
<<__EntryPoint>> function main(): void {
echo "*** Testing resource type with array_search() ***\n";
//file type resource
$file_handle = fopen(__FILE__, "r");

//directory type resource
$dir_handle = opendir( dirname(__FILE__) );

//store resources in array for comparison.
$resources = vec[$file_handle, $dir_handle];

// search for resouce type in the resource array
var_dump( array_search($file_handle, $resources, true) );
//checking for (int) type resource
var_dump( array_search((int)$dir_handle, $resources, true) );

/* Miscellenous input check  */
echo "\n*** Testing miscelleneos inputs with array_search() ***\n";
//matching "Good" in array(0,"hello"), result:true in loose type check
var_dump( array_search("Good", vec[0,"hello"]) );
//false in strict mode
var_dump( array_search("Good", vec[0,"hello"], TRUE) );

//matching integer 0 in array("this"), result:true in loose type check
var_dump( array_search(0, vec["this"]) );
// false in strict mode
var_dump( array_search(0, vec["this"]),TRUE );

//matching string "this" in array(0), result:true in loose type check
var_dump( array_search("this", vec[0]) );
// false in stric mode
var_dump( array_search("this", vec[0], TRUE) );

//checking for type FALSE in multidimensional array with loose checking, result:false in loose type check
var_dump( array_search(FALSE,
                   dict["a"=> TRUE, "b"=> TRUE,
                          0 => dict["c"=> TRUE, "d"=>TRUE]
                        ]
                  )
        );

//matching string having integer in beginning, result:true in loose type check
var_dump( array_search('123abc', vec[123]) );
var_dump( array_search('123abc', vec[123], TRUE) ); // false in strict mode

echo "Done\n";
}
