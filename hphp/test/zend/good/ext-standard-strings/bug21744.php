<?php
$test = <<< HERE
<a href="test?test\\!!!test">test</a>
<!-- test -->
HERE;

print strip_tags($test, '');
print strip_tags($test, '<a>');
?>