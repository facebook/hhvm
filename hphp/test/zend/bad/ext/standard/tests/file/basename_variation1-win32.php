<?php

$prefixes = array (
	
    // drive letters
	"A:/",
    "Z:/",
    "A:\\",

    // other prefixes
    "http://",
    "blah://",
	"blah:\\",
    "hostname:",

	// home directory ~
	"~/",
	"~\\",
);

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

foreach ($prefixes as $prefix) {
	foreach ($paths as $path) {
		$input = $prefix . $path;
		echo "basename for path $input is:\n";
		var_dump(basename($input));
	}
}

echo "\ndone\n";

?>