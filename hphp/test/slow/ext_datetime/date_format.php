<?php

date_default_timezone_set("America/Los_Angeles");

$dt = date_create("@1170288001");
var_dump(date_format($dt, "Y-m-d\\TH:i:s\\Z"));
var_dump(date_format($dt, "Y-m-d H:i:sZ"));
