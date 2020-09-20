<?hh
<<__EntryPoint>> function main(): void {
$paths = varray [

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

];

$suffixes = varray [

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
];

foreach ($paths as $path) {
	foreach ($suffixes as $suffix) {
		echo "basename for path $path, supplying suffix $suffix is:\n";
		var_dump(basename($path, $suffix));		
	}
}

echo "\ndone\n";
}
