<?
$MySQLi = new mysqli('localhost', 'my_user', 'my_password', 'my_db');
$MySQLi->multi_query('START TRANSACTION; SELECT * FROM (SELECT "Test") as `T1`; COMMIT;');
do {
  $Result = $MySQLi->store_result();
  var_dump($Result);
} while ($MySQLi->more_results() && $MySQLi->next_result());
?>