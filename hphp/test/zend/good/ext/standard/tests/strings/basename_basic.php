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
  varray["bar"],
  varray["/foo/bar"],
  varray["foo/bar"],
  varray["/bar"],

  /* simple paths with trailing slashes */
  varray["bar/"],
  varray["/bar/"],
  varray["/foo/bar/"],
  varray["foo/bar/"],
  varray["/bar/"],

  /* paths with suffix removal */
  varray["bar.gz", ".gz"],
  varray["bar.gz", "bar.gz"],
  varray["/foo/bar.gz", ".gz"],
  varray["foo/bar.gz", ".gz"],
  varray["/bar.gz", ".gz"],

  /* paths with suffix and trailing slashes with suffix removal*/
  varray["bar.gz/", ".gz"],
  varray["/bar.gz/", ".gz"],
  varray["/foo/bar.gz/", ".gz"],
  varray["foo/bar.gz/", ".gz"],
  varray["/bar.gz/", ".gz"],

  /* paths with basename only suffix, with suffix removal*/
  varray["/.gz", ".gz"],
  varray[".gz", ".gz"],
  varray["/foo/.gz", ".gz"],

  /* paths with basename only suffix & trailing slashes, with suffix removal*/
  varray[".gz/", ".gz"],
  varray["/foo/.gz/", ".gz"],
  varray["foo/.gz/", ".gz"],

  /* paths with binary value to check if the function is binary safe*/
  varray["foo".chr(0)."bar"],
  varray["/foo".chr(0)."bar"],
  varray["/foo".chr(0)."bar/"],
  varray["foo".chr(0)."bar/"],
  varray["foo".chr(0)."bar/test"],
  varray["/foo".chr(0)."bar/bar.gz", ".gz"],
  varray["/foo".chr(0)."bar/bar.gz"]
];

echo "*** Testing basic operations ***\n";
check_basename( $file_paths );

echo "Done\n";
}
