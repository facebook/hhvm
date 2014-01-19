<?php
$out = array();
$status = -1;
$php = getenv('TEST_PHP_EXECUTABLE');
if (substr(PHP_OS, 0, 3) != 'WIN') {
	exec($php . ' -n -r \'' 
	     . '$lengths = array(10,20000,10000,5,10000,3);'
	     . 'foreach($lengths as $length) {'
	     . '  for($i=0;$i<$length;$i++) print chr(65+$i % 27);'
	     . '  print "\n";'
	     . '}\'', $out, $status);
} else {
	exec($php . ' -n -r "' 
	     . '$lengths = array(10,20000,10000,5,10000,3);'
	     . 'foreach($lengths as $length) {'
	     . '  for($i=0;$i<$length;$i++) print chr(65+$i % 27);'
	     . '  print \\"\\n\\";'
	     . '}"', $out, $status);
}
for ($i=0;$i<6;$i++)
     print "md5(line $i)= " . md5($out[$i]) . " (length " .
strlen($out[$i]) . ")\n";
?>