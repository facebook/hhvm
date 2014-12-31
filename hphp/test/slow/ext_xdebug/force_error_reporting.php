<?php
error_reporting(E_ALL & (~E_USER_NOTICE));
trigger_error("This shouldn't show up", E_USER_NOTICE);
ini_set("xdebug.force_error_reporting", E_ALL);
trigger_error("This should show up", E_USER_NOTICE);
