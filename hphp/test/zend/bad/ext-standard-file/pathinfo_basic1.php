<?php
/* Prototype: mixed pathinfo ( string $path [, int $options] );
   Description: Returns information about a file path
*/

echo "*** Testing basic functions of pathinfo() ***\n";

$paths = array (
				'',
 			' ',
			'c:',
			'c:\\',
			'c:/',
			'afile',
			'c:\test\adir',
			'c:\test\adir\\',
			'/usr/include/arpa',
			'/usr/include/arpa/',
			'usr/include/arpa',
			'usr/include/arpa/',			
			'c:\test\afile',
			'c:\\test\\afile',
			'c://test//afile',
			'c:\test\afile\\',
			'c:\test\prog.exe',
			'c:\\test\\prog.exe',
			'c:/test/prog.exe',			
			'/usr/include/arpa/inet.h',
			'//usr/include//arpa/inet.h',
			'\\',
			'\\\\',
			'/',
			'//',
			'///',
			'/usr/include/arpa/inet.h',
			'c:\windows/system32\drivers/etc\hosts',
			'/usr\include/arpa\inet.h',
			'   c:\test\adir\afile.txt',
			'c:\test\adir\afile.txt   ',
			'   c:\test\adir\afile.txt   ',
			'   /usr/include/arpa/inet.h',
			'/usr/include/arpa/inet.h   ',
			'   /usr/include/arpa/inet.h   ',
			' c:',
			'		c:\test\adir\afile.txt',
			'/usr',
			'/usr/'
);

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
?>