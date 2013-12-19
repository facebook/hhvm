<?php
include "test_schema.inc";
$schema = '';
test_schema($schema,'type="xsd:unsignedLong"',0xffffffff);
echo "ok";
?>