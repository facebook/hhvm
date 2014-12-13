<?php
// This is testing our enhanced ini functionality, where we return things like
// arrays

// Interp mode testing will make sure we aren't always using the optimized
// version of ini_get (optimizedCallIniGet())

// First make sure we are working on normal primitive based bindings, where
// something easily convertible to string is returned
var_dump(ini_get("hhvm.hot_func_count"));
var_dump(ini_get("hhvm.stats.slot_duration"));

// And now the collection-y type settings
var_dump(ini_get("hhvm.server.allowed_exec_cmds"));
var_dump(ini_get("hhvm.env_variables"));
var_dump(ini_get("hhvm.server_variables"));
