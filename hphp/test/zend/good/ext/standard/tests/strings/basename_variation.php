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
$file_path_variations = vec[
  /* paths with shortcut home dir char, with suffix variation */
  vec["~/home/user/bar"],
  vec["~/home/user/bar", ""],
  vec["~/home/user/bar", ' '],
  vec["~/home/user/bar.tar", ".tar"],
  vec["~/home/user/bar.tar", "~"],
  vec["~/home/user/bar.tar/", "~"],
  vec["~/home/user/bar.tar/", ""],
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

  /* path with spaces */
  vec[" "],
  vec[' '],

  /* empty paths */
  vec[""],
  vec[''],
];

echo "*** Testing possible variations in path and suffix ***\n";
check_basename( $file_path_variations );

echo "Done\n";
}
