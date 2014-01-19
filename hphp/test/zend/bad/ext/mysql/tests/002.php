<?php
require_once('connect.inc');

if (!$link = my_mysql_connect($host, $user, $passwd, $db, $port, $socket))
	printf("[001] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
		$host, $user, $db, $port, $socket);

var_dump($link);

var_dump(mysql_query('DROP TABLE IF EXISTS test'));

var_dump(mysql_query('CREATE TABLE test(col1 int PRIMARY KEY, col2 varchar(50), col3 char(5))'));

var_dump(mysql_query("INSERT INTO test(col1, col2, col3) VALUES(1, 'foo', 'bar'),(2, 'foo', 'bar')"));

var_dump($res = mysql_query('SELECT col1, col2, col3 FROM test ORDER BY col1 ASC'));

while ($data = mysql_fetch_array($res, MYSQL_ASSOC)) {
	var_dump($data);
}

mysql_free_result($res);
mysql_close($link);

print "done!";
?>
<?php
require_once("clean_table.inc");
?>