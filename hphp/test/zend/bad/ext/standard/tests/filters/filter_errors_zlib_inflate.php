<?php
require 'filter_errors.inc';
filter_errors_test('zlib.inflate', gzencode(b'42'));
?>