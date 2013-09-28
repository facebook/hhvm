<?php
header("X-foo: e\n foo");
header("X-Foo6: e\0Set-Cookie: ID=\n123\n d");
echo 'foo';
?>