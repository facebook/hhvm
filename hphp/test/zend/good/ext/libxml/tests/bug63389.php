<?php
$fp = fopen("php://input", "r");
libxml_set_streams_context($fp);
libxml_set_streams_context("a");
echo "okey";
?>