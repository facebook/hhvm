<?php
setlocale(LC_ALL, 'fr_FR.ISO8859-1', 'fr_FR');
$table = array("AB" => "Alberta",
"BC" => "Colombie-Britannique",
"MB" => "Manitoba",
"NB" => "Nouveau-Brunswick",
"NL" => "Terre-Neuve-et-Labrador",
"NS" => "Nouvelle-\xEF\xBF\xBDcosse",
"ON" => "Ontario",
"PE" => "\xEF\xBF\xBDle-du-Prince-\xEF\xBF\xBDdouard",
"QC" => "Qu\xEF\xBF\xBDbec",
"SK" => "Saskatchewan",
"NT" => "Territoires du Nord-Ouest",
"NU" => "Nunavut",
"YT" => "Territoire du Yukon");
asort($table, SORT_LOCALE_STRING);

#ASCII-ize
foreach($table as &$val) {
  $val = urlencode($val);
}
unset($val);
var_dump($table);
