<?php

preg_match("\0//i", "");
preg_match("/\0/i", "");
preg_match("//\0i", "");
preg_match("//i\0", "");
preg_match("/\\\0/i", "");

preg_match("\0[]i", "");
preg_match("[\0]i", "");
preg_match("[]\0i", "");
preg_match("[]i\0", "");
preg_match("[\\\0]i", "");

preg_replace("/foo/e\0/i", "echo('Eek');", "");

?>