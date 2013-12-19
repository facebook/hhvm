<?php
mb_internal_encoding("utf-8");
mb_parse_str("a=%fc", $dummy);
var_dump(mb_http_input());
?>