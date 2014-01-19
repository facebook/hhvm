<?php
date_default_timezone_set('GMT');
echo strtotime("2006-05-12 13:00:01 America/New_York"), "\n";
echo strtotime("2006-05-12 13:00:00 America/New_York"), "\n";
echo strtotime("2006-05-12 12:59:59 America/New_York"), "\n";
echo strtotime("2006-05-12 12:59:59 GMT"), "\n";
?>