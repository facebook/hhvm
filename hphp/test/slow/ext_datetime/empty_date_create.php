<?php
date_default_timezone_set("GMT");
var_dump(date_create());
var_dump(date_create(""));
var_dump(date_create(null));
var_dump(date_create(null, timezone_open("UTC")));
