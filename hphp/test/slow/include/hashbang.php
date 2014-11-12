#!/bin/env php
<?php

echo "included\n";

if (!isset($_ENV['DID_RUN_HASHBANG_PHP_ONCE'])) {
  $_ENV['DID_RUN_HASHBANG_PHP_ONCE'] = '1';
  require __FILE__;
}
