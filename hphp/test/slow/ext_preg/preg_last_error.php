<?php
preg_match("/(?:\\D+|<\\d+>)*[!?]/", "foobar foobar foobar");
var_dump(preg_last_error() === 2);
