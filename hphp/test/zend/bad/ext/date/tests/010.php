<?php
date_default_timezone_set('UTC');
  $timezone_abbreviations = timezone_abbreviations_list();
  var_dump($timezone_abbreviations["utc"]);
  echo "Done\n";
?>