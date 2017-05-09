<?php
   system("mkdir -p /tmp/lv11/sourceroot/", $result);
   system("mkdir -p /tmp/lv11/conf/", $result);
 
   file_put_contents("/tmp/lv11/sourceroot/test.php", "<?php 
   require('../conf/test.php');  echo 'this is in /tmp/lv11/sourceroot/test.php\n';");
   file_put_contents("/tmp/lv11/conf/test.php", "<?php 
   echo 'this is in /tmp/lv11/conf/test.php\n';");
 
   system("ln -s /tmp/lv11/sourceroot/ /tmp/lv12", $result);
 
   require('/tmp/lv12/test.php');
 
   system("rm -rf /tmp/lv12", $result);
   system("rm -rf /tmp/lv11", $result);