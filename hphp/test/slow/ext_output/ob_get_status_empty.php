<?php

ob_start();
var_dump((bool)ob_get_status(false));
var_dump(count(ob_get_status(true)));
