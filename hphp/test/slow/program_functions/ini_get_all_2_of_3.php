<?php
require_once __DIR__ . '/ini_get_all_impl.inc';
run_tests("hhvm.jit_profile_requests",
          "hhvm.server.force_server_name_to_header");
