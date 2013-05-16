<?php
$resource = tmpfile();

imagesetthickness('string', 5);
imagesetthickness(array(), 5);
imagesetthickness($resource, 5);
?>