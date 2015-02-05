<?php
// Tests for segfault if no connection available

@mysql_real_escape_string("");

echo "ok";
