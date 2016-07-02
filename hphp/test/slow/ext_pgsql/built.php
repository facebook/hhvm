<?php

var_dump(function_exists('pg_connect'));
var_dump(in_array('pgsql', PDO::getAvailableDrivers()));
