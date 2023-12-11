<?hh
/*
 * proto string basename(string path [, string suffix])
 * Function is implemented in ext/standard/string.c
 */

function check_basename( $path_arrays ) :mixed{
  $loop_counter = 1;
  foreach ($path_arrays as $path) {
    echo "\n--Iteration $loop_counter--\n"; $loop_counter++;
    if( 1 == count($path) ) {
      // no suffix provided
      try { var_dump( basename($path[0]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    } else {
      // path as well as suffix provided,
      try { var_dump( basename($path[0], $path[1]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
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

$file_path_variations = varray [
  /* paths with shortcut home dir char, with suffix variation */
  vec["~/home/user/bar"],
  vec["~/home/user/bar", ""],
  vec["~/home/user/bar", NULL],
  vec["~/home/user/bar", ' '],
  vec["~/home/user/bar.tar", ".tar"],
  vec["~/home/user/bar.tar", "~"],
  vec["~/home/user/bar.tar/", "~"],
  vec["~/home/user/bar.tar/", ""],
  vec["~/home/user/bar.tar", NULL],
  vec["~/home/user/bar.tar", ''],
  vec["~/home/user/bar.tar", " "],

  /* paths with hostname:dir notation, with suffix variation */
  vec["hostname:/home/usr/bar.tar"],
  vec["hostname:/home/user/bar.tar", "home"],
  vec["hostname:/home/user/tbar.gz", "bar.gz"],
  vec["hostname:/home/user/tbar.gz", "/bar.gz"],
  vec["hostname:/home/user/tbar.gz", "/bar.gz/"],
  vec["hostname:/home/user/tbar.gz/", "/bar.gz/"],
  vec["hostname:/home/user/tbar.gz/", "/bar.gz/"],
  vec["hostname:/home/user/My Pics.gz/", "/bar.gz/"],
  vec["hostname:/home/user/My Pics.gz/", "Pics.gz/"],
  vec["hostname:/home/user/My Pics.gz/", "Pics.gz"],
  vec["hostname:/home/user/My Pics.gz/", ".gz"],
  vec["hostname:/home/user/My Pics.gz/"],
  vec["hostname:/home/user/My Pics.gz/", NULL],
  vec["hostname:/home/user/My Pics.gz/", ' '],
  vec["hostname:/home/user/My Pics.gz/", ''],
  vec["hostname:/home/user/My Pics.gz/", "My Pics.gz"],

  /* paths with numeirc strings */
  vec["10.5"],
  vec["10.5", ".5"],
  vec["10.5", "10.5"],
  vec["10"],
  vec["105", "5"],
  vec["/10.5"],
  vec["10.5/"],
  vec["10/10.gz"],
  vec["0"],
  vec['0'],

  /* paths and suffix given as same */
  vec["bar.gz", "bar.gz"],
  vec["/bar.gz", "/bar.gz"],
  vec["/bar.gz/", "/bar.gz/"],
  vec[" ", " "],
  vec[' ', ' '],
  vec[NULL, NULL],

  /* path with spaces */
  vec[" "],
  vec[' '],

  /* empty paths */
  vec[""],
  vec[''],
  vec[NULL]
];

echo "*** Testing basic operations ***\n";
check_basename( $file_paths );

echo "\n*** Testing possible variations in path and suffix ***\n";
check_basename( $file_path_variations );

echo "\n*** Testing error conditions ***\n";
// zero arguments
try { var_dump( basename() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than expected no. of arguments
try { var_dump( basename(sys_get_temp_dir()."/bar.gz", ".gz", ".gz") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// passing invalid type arguments
$object = new stdClass;
try { var_dump( basename( vec["string/bar"] ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( vec["string/bar"], "bar" ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( "bar", vec["string/bar"] ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( $object, "bar" ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( $object ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( $object, $object ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( "bar", $object ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
