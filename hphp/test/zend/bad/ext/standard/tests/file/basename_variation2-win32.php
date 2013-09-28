<?php

$paths = array (

	"foo",
	"foo/",
    "foo\\",    
    "foo.bar",
    "foo.bar/",
    "foo.bar\\",
    "dir/foo.bar",
    "dir\\foo.bar",
    "dir with spaces/foo.bar",
    "dir with spaces\\foo.bar",

);

$suffixes = array (

	".bar",
	".b",
    ".",
    " ",
    "foo",
    "foo.bar",
    "foo/bar",
    "foo\\bar",
    "/",
    "\\",	
);

foreach ($paths as $path) {
	foreach ($suffixes as $suffix) {
		echo "basename for path $path, supplying suffix $suffix is:\n";
		var_dump(basename($path, $suffix));		
	}
}

echo "\ndone\n";

?>