<?php

$dt = date_create("@1170288001");
var_dump(date_format($dt, "Y-m-d\\TH:i:s\\Z") === "2007-02-01T00:00:01Z");
var_dump(date_format($dt, "Y-m-d H:i:sZ") === "2007-02-01 00:00:01-28800");
