<?php
header("X-foo: e\n foo");
header("X-Foo6: e\rSet-Cookie: ID=123\n d");
echo 'foo';
?>