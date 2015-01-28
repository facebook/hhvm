<?php

var_dump(timezone_name_from_abbr('', -18000, 0)); // string(16) "America/New_York"
var_dump(timezone_name_from_abbr('', -18000, 1)); // string(15) "America/Chicago"
