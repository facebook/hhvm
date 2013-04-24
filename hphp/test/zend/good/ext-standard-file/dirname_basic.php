<?php
/* Prototype  : string dirname(string path)
 * Description: Returns the directory name component of the path 
 * Source code: ext/standard/string.c
 * Alias to functions: 
 */

echo "*** Testing dirname() : basic functionality ***\n";


// Initialise all required variables
$paths = array(
 			'',
 			' ',
			'c:',
			'c:\\',
			'c:/',
			'afile',
			'c:\test\afile',
			'c:\\test\\afile',
			'c://test//afile',
			'c:\test\afile\\',
			'/usr/lib/locale/en_US',
			'//usr/lib//locale/en_US',
			'\\',
			'\\\\',
			'/',
			'//',
			'///',
			'/usr/lib/locale/en_US/',
			'c:\windows/system32\drivers/etc\hosts',
			'/usr\lib/locale\en_US',
			'   c:\test\adir\afile.txt',
			'c:\test\adir\afile.txt   ',
			'   c:\test\adir\afile.txt   ',
			'   /usr/lib/locale/en_US',
			'/usr/lib/locale/en_US   ',
			'   /usr/lib/locale/en_US   ',
			' c:',
			'		c:\test\adir\afile.txt',
			'/usr',
			'/usr/'			
			);

foreach ($paths as $path) {
	var_dump( dirname($path) );
}

?>
===DONE===