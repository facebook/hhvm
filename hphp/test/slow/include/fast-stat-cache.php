<?php

$file = tempnam('/tmp', 'test-fast-stat-cache');
file_put_contents($file, "<?php\necho 'Old';");
include($file);
unlink($file);
sleep(1);
file_put_contents($file, "<?php\necho 'New';");
include($file);
sleep(1);
include($file);
