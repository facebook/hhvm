<?php
putenv("foo=ab");
putenv("bar=c");
var_dump(getenv("foo"));
var_dump(getenv("bar"));
var_dump(getenv("thisvardoesnotexist"));
?>