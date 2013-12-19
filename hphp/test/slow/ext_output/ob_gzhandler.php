<?php

ob_start('ob_gzhandler'); // For now this will emit a warning just like if zend was compiled withot zlib
echo "hi";
ob_get_flush();
