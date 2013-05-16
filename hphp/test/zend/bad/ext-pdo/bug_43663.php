<?php
class test extends PDO{
    function __call($name, array $args) {
        echo "Called $name in ".__CLASS__."\n";
    }
    function foo() {
        echo "Called foo in ".__CLASS__."\n";
    }
}

if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';

$a = new test('sqlite::memory:');
$a->foo();
$a->bar();