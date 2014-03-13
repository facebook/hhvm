<?php
// Explicit test as Composer depends on it

var_dump(version_compare('1.1.0', LIBXSLT_DOTTED_VERSION));
var_dump(version_compare('2.0.0', LIBXSLT_DOTTED_VERSION));
