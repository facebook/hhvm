<?php
DateTime::createFromFormat('s', '0');
$lastErrors = DateTime::getLastErrors();
print_r($lastErrors['errors'][0]);
?>