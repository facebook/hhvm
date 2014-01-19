<?php

// clearstatcache do nothing, but there should no warnnings
var_dump(clearstatcache());
var_dump(clearstatcache(true));
var_dump(clearstatcache(false, "test.php"));
