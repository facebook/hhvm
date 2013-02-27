<?php

print "Test begin\n";

require_once 'ReqOnce_b.php';
print "Between require_once invocations\n";
require_once 'ReqOnce_b.php';

print "Test end\n";
