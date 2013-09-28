<?php

var_dump(date_format(date_create("2006-12-12"), "Y-m-d H:i:s") ===
  "2006-12-12 00:00:00");
var_dump(date_format(date_create("@1170288001"), "Y-m-d H:i:s") ===
  "2007-02-01 00:00:01");
