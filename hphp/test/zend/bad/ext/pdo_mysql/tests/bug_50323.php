<?php
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');

    function changeDSN($original, $new_options) {
        $old_options = array();
        $dsn = substr($original,
                strpos($original, ':') + 1,
                strlen($original));

        // no real parser - any excotic setting can fool us
        $parts = explode(';', $dsn);
        foreach ($parts as $k => $v) {
            $tmp = explode('=', $v);
            if (count($tmp) == 2)
                    $old_options[$tmp[0]] = $tmp[1];
        }

        $options = $old_options;
        foreach ($new_options as $k => $v)
            $options[$k] = $v;

        $dsn = 'mysql:';
        foreach ($options as $k => $v)
            $dsn .= sprintf('%s=%s;', $k, $v);

        $dsn = substr($dsn, 0, strlen($dsn) -1);

        return $dsn;
    }


if (1 === @$db->exec('CREATE DATABASE `crazy;dbname`')) {
    $dsn = changeDSN(getenv('PDOTEST_DSN'), array('dbname' => 'crazy;;dbname'));
    $user = getenv('PDOTEST_USER');
    $pass = getenv('PDOTEST_PASS');
    
    new PDO($dsn, $user, $pass);
}
echo 'done!';
?>
<?php
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');

@$db->exec('DROP DATABASE IF EXISTS `crazy;dbname`');
?>