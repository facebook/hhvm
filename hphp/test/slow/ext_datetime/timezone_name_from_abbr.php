<?php

var_dump(timezone_name_from_abbr('', -18000, -2));
var_dump(timezone_name_from_abbr('', -18000, -1));
var_dump(timezone_name_from_abbr('', -18000, 0));
var_dump(timezone_name_from_abbr('', -18000, 1));
var_dump(timezone_name_from_abbr('', -18000, 2));
var_dump(timezone_name_from_abbr('EST', -18000, 2));
var_dump(timezone_name_from_abbr('EST', -18000, 2));
var_dump(timezone_name_from_abbr('PST', -18000, 2));
var_dump(timezone_name_from_abbr('PST', -18000, 2));
