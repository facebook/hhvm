<?php

require_once('test_base.inc');

requestAll(array(
    "test_fastcgi_finish_request.php?error=0",
    "test_fastcgi_finish_request.php?error=1",
));
