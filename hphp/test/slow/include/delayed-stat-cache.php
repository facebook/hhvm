<?php

$file = tempnam('/tmp', 'test-delayed-stat-cache');
file_put_contents($file, "<?php\necho 'Old';");
include($file);
file_put_contents($file, "<?php\necho 'New';");
touch($file, filemtime($file) + 1); // modify the file mtime to make sure stat changed
include($file);
usleep(1100000);
include($file);
