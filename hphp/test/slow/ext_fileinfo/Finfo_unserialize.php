<?php

$f = new finfo(FILEINFO_MIME_TYPE);
echo unserialize(serialize($f))->file(__FILE__), "\n";
$f->set_flags(FILEINFO_NONE);
echo unserialize(serialize($f))->file(__FILE__), "\n";
