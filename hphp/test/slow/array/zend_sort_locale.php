<?hh

<<__EntryPoint>>
function main_zend_sort_locale() :mixed{
setlocale(LC_ALL, 'fr_FR.ISO8859-1', 'fr_FR');
$table = dict["AB" => "Alberta",
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
"YT" => "Territoire du Yukon"];
asort(inout $table, SORT_LOCALE_STRING);

//ASCII-ize
foreach($table as $key => $val) {
  $table[$key] = urlencode($val);
}
unset($val);
var_dump($table);
}
