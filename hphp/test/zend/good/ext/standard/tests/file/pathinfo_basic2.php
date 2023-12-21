<?hh
/* Prototype: mixed pathinfo ( string $path [, int $options] );
   Description: Returns information about a file path
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing basic functions of pathinfo() ***\n";

$paths = vec[
            'c:\..\dir1',
            'c:\test\..\test2\.\adir\afile.txt',
            '/usr/include/../arpa/./inet.h',
            'c:\test\adir\afile..txt',
            '/usr/include/arpa/inet..h',
            'c:\test\adir\afile.',
            '/usr/include/arpa/inet.',
            '/usr/include/arpa/inet,h',
            'c:afile.txt',
            '..\.\..\test\afile.txt',
            '.././../test/afile',
            '.',
            '..',
            '...',
            '/usr/lib/.../afile'

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
