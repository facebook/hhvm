<?hh

function main() {
  $d = new DateTimeZone("America/New_York");
  var_dump($d->getLocation());
  $invalid_date = "2OO9-02--27 10:00?00.5";
  date_parse($invalid_date);
  var_dump(DateTime::getLastErrors());
}

main();
