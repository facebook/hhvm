<?php
$result = exec('cd 1:\non_existant; dir nonexistant');
echo "$result";
system('cd 1:\non_existant; dir nonexistant');
?>