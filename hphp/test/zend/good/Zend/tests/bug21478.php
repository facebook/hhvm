<?php
class debugfilter extends php_user_filter {
  function filter($in, $out, &$consumed, $closing) {
    while ($bucket = stream_bucket_make_writeable($in)) {
      $bucket->data = strtoupper($bucket->data);
      stream_bucket_append($out, $bucket);
      $consumed += strlen($bucket->data);
    }
    return PSFS_PASS_ON;
  }
}

stream_filter_register("myfilter","debugfilter");

$fp = fopen(dirname(__FILE__)."/test.txt","w");
stream_filter_append($fp, "myfilter");
stream_filter_append($fp, "myfilter");
stream_filter_append($fp, "myfilter");
fwrite($fp, "This is a test.\n");
print "Done.\n";
fclose($fp);
// Uncommenting the following 'print' line causes the segfault to stop occuring
// print "2\n";  
readfile(dirname(__FILE__)."/test.txt");
unlink(dirname(__FILE__)."/test.txt");
?>
