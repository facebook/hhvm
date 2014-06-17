<?php
$msqi = new mysqli('localhost', 'my_user', 'my_password', 'my_db');
$sql_str = 'START TRANSACTION; SELECT * FROM (SELECT "Test") as `T1`; COMMIT;';
$msqi->multi_query($sql_str);
do {
  $result = $msqi->store_result();
  var_dump($result);
} while ($msqi->more_results() && $msqi->next_result());
