<?hh
<<__EntryPoint>>
function main_entry(): void {
  setlocale(LC_ALL, 'fr_FR.ISO8859-1', 'fr_FR');
  $table = array("AB" => "Alberta",
  "BC" => "Colombie-Britannique",
  "MB" => "Manitoba",
  "NB" => "Nouveau-Brunswick",
  "NL" => "Terre-Neuve-et-Labrador",
  "NS" => "Nouvelle-�cosse",
  "ON" => "Ontario",
  "PE" => "�le-du-Prince-�douard",
  "QC" => "Qu�bec",
  "SK" => "Saskatchewan",
  "NT" => "Territoires du Nord-Ouest",
  "NU" => "Nunavut",
  "YT" => "Territoire du Yukon");
  asort(&$table, SORT_LOCALE_STRING);
  var_dump($table);
}
