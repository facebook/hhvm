<?hh

// bad
include 'test.php';
include_once 'test.php';

// ok
require 'test.php';
require_once 'test.php';
