<?php
$fname = tempnam(__DIR__, 'xmlout');
$writer = xmlwriter_open_uri($fname);
xmlwriter_flush($writer);
print file_exists($fname) ? "written\n" : "no file\n";
unlink($fname);
