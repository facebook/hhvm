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
       try { var_dump( basename($path[0]) ); } catch (Exception $e) { var_dump($e->getMessage()); }
     } else {
       // path as well as suffix provided,
       try { var_dump( basename($path[0], $path[1]) ); } catch (Exception $e) { var_dump($e->getMessage()); }
     }
   }
}

<<__EntryPoint>> function main(): void {
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

echo "*** Testing possible variations in path and suffix ***\n";
check_basename( $file_path_variations );

echo "Done\n";
}
