<?php
header("X-Foo1: a");
header("X-Foo2: b\n ");
header("X-Foo3: c\r\n ");
header("X-Foo4: d\r ");
header("X-Foo5: e\rSet-Cookie: ID=123");
echo 'foo';
?>