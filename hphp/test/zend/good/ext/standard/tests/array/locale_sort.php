<?hh
<<__EntryPoint>>
function main_entry(): void {
  setlocale(LC_ALL, 'fr_FR.ISO8859-1', 'fr_FR');
  $table = dict["AB" => "Alberta",
  "BC" => "Colombie-Britannique",
  "MB" => "Manitoba",
  "NB" => "Nouveau-Brunswick",
  "NL" => "Terre-Neuve-et-Labrador",
  "NS" => "Nouvelle-Écosse",
  "ON" => "Ontario",
  "PE" => "Île-du-Prince-Édouard",
  "QC" => "Québec",
  "SK" => "Saskatchewan",
  "NT" => "Territoires du Nord-Ouest",
  "NU" => "Nunavut",
  "YT" => "Territoire du Yukon"];
  asort(inout $table, SORT_LOCALE_STRING);
  var_dump($table);
}
