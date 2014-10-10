<?php

trigger_error("Test notice", E_USER_NOTICE);
trigger_error("Test deprecated", E_USER_DEPRECATED);
ini_set("xdebug.halt_level", E_ALL);
trigger_error("Test deprecated", E_USER_DEPRECATED);
trigger_error("Test notice", E_USER_NOTICE);
