<?php

file_put_contents('foo.php', "<?php \n\$i=1;\n");
include 'foo.php';
echo $i;
sleep(2); // just to make sure that timestamp is different 
file_put_contents('foo.php', "<?php \n\$i=2;\n");
include 'foo.php';
echo $i;

?>
