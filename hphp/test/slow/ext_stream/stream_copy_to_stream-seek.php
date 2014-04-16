<?php

// Create two temporary files.
$file1 = tmpfile();
$file2 = tmpfile();
fwrite($file1, 'input');
fwrite($file2, 'before');

// Seek to specific position in destination.
rewind($file1);
fseek($file2, 2);

stream_copy_to_stream($file1, $file2);

// Show us all of file2.
echo stream_get_contents($file2, -1, 0);

// Cleanup.
fclose($file1);
fclose($file2);
