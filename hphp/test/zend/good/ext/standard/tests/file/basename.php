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

$file_path_variations = varray [
  /* paths with shortcut home dir char, with suffix variation */
  varray["~/home/user/bar"],
  varray["~/home/user/bar", ""],
  varray["~/home/user/bar", NULL],
  varray["~/home/user/bar", ' '],
  varray["~/home/user/bar.tar", ".tar"],
  varray["~/home/user/bar.tar", "~"],
  varray["~/home/user/bar.tar/", "~"],
  varray["~/home/user/bar.tar/", ""],
  varray["~/home/user/bar.tar", NULL],
  varray["~/home/user/bar.tar", ''],
  varray["~/home/user/bar.tar", " "],

  /* paths with hostname:dir notation, with suffix variation */
  varray["hostname:/home/usr/bar.tar"],
  varray["hostname:/home/user/bar.tar", "home"],
  varray["hostname:/home/user/tbar.gz", "bar.gz"],
  varray["hostname:/home/user/tbar.gz", "/bar.gz"],
  varray["hostname:/home/user/tbar.gz", "/bar.gz/"],
  varray["hostname:/home/user/tbar.gz/", "/bar.gz/"],
  varray["hostname:/home/user/tbar.gz/", "/bar.gz/"],
  varray["hostname:/home/user/My Pics.gz/", "/bar.gz/"],
  varray["hostname:/home/user/My Pics.gz/", "Pics.gz/"],
  varray["hostname:/home/user/My Pics.gz/", "Pics.gz"],
  varray["hostname:/home/user/My Pics.gz/", ".gz"],
  varray["hostname:/home/user/My Pics.gz/"],
  varray["hostname:/home/user/My Pics.gz/", NULL],
  varray["hostname:/home/user/My Pics.gz/", ' '],
  varray["hostname:/home/user/My Pics.gz/", ''],
  varray["hostname:/home/user/My Pics.gz/", "My Pics.gz"],

  /* paths with numeirc strings */
  varray["10.5"],
  varray["10.5", ".5"],
  varray["10.5", "10.5"],
  varray["10"],
  varray["105", "5"],
  varray["/10.5"],
  varray["10.5/"],
  varray["10/10.gz"],
  varray["0"],
  varray['0'],

  /* paths and suffix given as same */
  varray["bar.gz", "bar.gz"],
  varray["/bar.gz", "/bar.gz"],
  varray["/bar.gz/", "/bar.gz/"],
  varray[" ", " "],
  varray[' ', ' '],
  varray[NULL, NULL],

  /* path with spaces */
  varray[" "],
  varray[' '],

  /* empty paths */
  varray[""],
  varray[''],
  varray[NULL]
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
try { var_dump( basename( varray["string/bar"] ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( varray["string/bar"], "bar" ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( "bar", varray["string/bar"] ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( $object, "bar" ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( $object ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( $object, $object ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( basename( "bar", $object ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
