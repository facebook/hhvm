<?php

$empty = tempnam(sys_get_temp_dir(), 'Empty');
$empty_php = tempnam(sys_get_temp_dir(), 'EmptyPHP');

file_put_contents($empty_php, '<?php ');

include($empty);
include($empty_php);

require($empty);
require($empty_php);

@unlink($empty);
@unlink($empty_php);
