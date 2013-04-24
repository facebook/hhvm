<?php
header("X-foo: e\r\n foo");
header("X-foo: e\r\nfoo");
echo 'foo';
?>