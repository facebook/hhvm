<?php
$sessdir = dirname(__FILE__).'/sessions/';
@rmdir($sessdir);
mkdir($sessdir);
$save_path = '0;0777;'.$sessdir;
umask(0);
session_save_path($save_path);
session_start();
echo "hello world\n";
session_write_close();

foreach (glob($sessdir. "*") as $sessfile) {
  var_dump(decoct(fileperms($sessfile)));
  unlink($sessfile);
}
rmdir($sessdir);