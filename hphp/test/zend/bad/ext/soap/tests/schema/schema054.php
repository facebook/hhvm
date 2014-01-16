<?php
include "test_schema.inc";
$schema = '';
test_schema($schema,'type="apache:Map" xmlns:apache="http://xml.apache.org/xml-soap"',array('a'=>123,'b'=>123.5));
echo "ok";
?>