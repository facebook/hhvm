<?php

$host   = getenv("MYSQL_TEST_HOST");
$port   = getenv("MYSQL_TEST_PORT");
$user   = getenv("MYSQL_TEST_USER");
$passwd = getenv("MYSQL_TEST_PASSWD");
$db     = getenv("MYSQL_TEST_DB");

try {
    $dbh = new PDO(
        'mysql:host='.$host.':'.$port.';'.
        $user,
        $passwd
    );

    $dbh->query( "CREATE DATABASE IF NOT EXISTS $db" );
    $dbh->query( "use $db" );
    $dbh->query( "DROP TABLE IF EXISTS typed_results_true" );
    $dbh->query( "CREATE TABLE typed_results_true (
                    intField   INT(6) UNSIGNED NOT NULL DEFAULT 0,
                    floatField FLOAT NOT NULL DEFAULT 0,
                    strField   VARCHAR(255) NOT NULL DEFAULT ''
                 ) ENGINE=InnoDB" );

    $dbh->query( "INSERT INTO typed_results_true VALUES (
                    123, 1234.5674323, '123214qwe')" );

    $stmt = $dbh->query(
        'SELECT *, floatField / intField AS calc FROM typed_results_true',
        PDO::FETCH_ASSOC
    );

    $row = $stmt->fetchAll();
    var_dump($row);

} catch (Exception $ex) {
    print $ex->getMessage();
} finally {
    $dbh->query( "DROP TABLE typed_results_true" );
}
