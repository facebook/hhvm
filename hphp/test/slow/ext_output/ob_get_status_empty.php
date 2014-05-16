<?php

ob_start();
var_dump(ob_get_status(false));
var_dump(ob_get_status(true));
