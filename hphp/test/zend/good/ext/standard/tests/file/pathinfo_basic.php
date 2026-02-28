<?hh
/* Prototype: mixed pathinfo ( string $path [, int $options] );
   Description: Returns information about a file path
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing basic functions of pathinfo() ***\n";

$file_path = dirname(__FILE__);

$paths = vec[
  /* Testing basic file notation */
  "$file_path/foo/symlink.link",
  "www.example.co.in",
  "/var/www/html/example.html",
  "/dir/test.tar.gz",

  /* Testing a file with trailing slash */
  "$file_path/foo/symlink.link/",

  /* Testing file with double slashes */
  "$file_path/foo//symlink.link",
  "$file_path/foo//symlink.link",
  "$file_path/foo//symlink.link//",

  /* Testing file with trailing double slashes */
  "$file_path/foo/symlink.link//",

  /* Testing Binary safe files */
  "$file_path/foo".chr(47)."symlink.link",
  "$file_path".chr(47)."foo/symlink.link",
  "$file_path".chr(47)."foo".chr(47)."symlink.link",
  b"$file_path/foo/symlink.link",

  /* Testing directories */
  ".",  // current directory
  "$file_path/foo/",
  "$file_path/foo//",
  "$file_path/../foo/",
  "../foo/bar",
  "./foo/bar",
  "//foo//bar//",

  /* Testing with homedir notation */
  "~/PHP/php5.2.0/",
  
  /* Testing normal directory notation */
  "/home/example/test/",
  "http://httpd.apache.org/core.html#acceptpathinfo"
];

$counter = 1;
/* loop through $paths to test each $path in the above array */
foreach($paths as $path) {
  echo "-- Iteration $counter --\n";
  var_dump( pathinfo($path, PATHINFO_DIRNAME) );
  var_dump( pathinfo($path, PATHINFO_BASENAME) );
  var_dump( pathinfo($path, PATHINFO_EXTENSION) );
  var_dump( pathinfo($path, PATHINFO_FILENAME) );
  var_dump( pathinfo($path) );
  $counter++;
}

echo "Done\n";
}
