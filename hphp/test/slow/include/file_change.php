<?php

$file = tempnam('/tmp', 'test-file-change');
$fd = fopen($file, 'w+');
fwrite($fd, "<?php\n");
echo "Include when file is incomplete\n";
include($file);
fwrite($fd, "var_dump('Hello World');\n");
fclose($fd);
echo "Include when file is complete\n";
include($file);
