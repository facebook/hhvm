<?php
var_dump(parse_url('http://user:pass@host'));
var_dump(parse_url('//user:pass@host'));
var_dump(parse_url('//user@host'));
?>