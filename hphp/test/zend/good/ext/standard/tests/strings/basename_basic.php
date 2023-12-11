<?hh
/* Prototype: string basename ( string $path [, string $suffix] );
   Description: Given a string containing a path to a file,
                this function will return the base name of the file.
                If the filename ends in suffix this will also be cut off.
*/

function check_basename( $path_arrays ) :mixed{
   $loop_counter = 1;
   foreach ($path_arrays as $path) {
     echo "\n--Iteration $loop_counter--\n"; $loop_counter++;
     if( 1 == count($path) ) {
       // no suffix provided
       var_dump( basename($path[0]) );
     } else {
       // path as well as suffix provided,
       var_dump( basename($path[0], $path[1]) );
     }
   }
}

<<__EntryPoint>> function main(): void {
$file_paths = varray [
  /* simple paths */
  vec["bar"],
  vec["/foo/bar"],
  vec["foo/bar"],
  vec["/bar"],

  /* simple paths with trailing slashes */
  vec["bar/"],
  vec["/bar/"],
  vec["/foo/bar/"],
  vec["foo/bar/"],
  vec["/bar/"],

  /* paths with suffix removal */
  vec["bar.gz", ".gz"],
  vec["bar.gz", "bar.gz"],
  vec["/foo/bar.gz", ".gz"],
  vec["foo/bar.gz", ".gz"],
  vec["/bar.gz", ".gz"],

  /* paths with suffix and trailing slashes with suffix removal*/
  vec["bar.gz/", ".gz"],
  vec["/bar.gz/", ".gz"],
  vec["/foo/bar.gz/", ".gz"],
  vec["foo/bar.gz/", ".gz"],
  vec["/bar.gz/", ".gz"],

  /* paths with basename only suffix, with suffix removal*/
  vec["/.gz", ".gz"],
  vec[".gz", ".gz"],
  vec["/foo/.gz", ".gz"],

  /* paths with basename only suffix & trailing slashes, with suffix removal*/
  vec[".gz/", ".gz"],
  vec["/foo/.gz/", ".gz"],
  vec["foo/.gz/", ".gz"],

  /* paths with binary value to check if the function is binary safe*/
  vec["foo".chr(0)."bar"],
  vec["/foo".chr(0)."bar"],
  vec["/foo".chr(0)."bar/"],
  vec["foo".chr(0)."bar/"],
  vec["foo".chr(0)."bar/test"],
  vec["/foo".chr(0)."bar/bar.gz", ".gz"],
  vec["/foo".chr(0)."bar/bar.gz"]
];

echo "*** Testing basic operations ***\n";
check_basename( $file_paths );

echo "Done\n";
}
