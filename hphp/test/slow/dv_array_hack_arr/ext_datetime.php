<?hh

<<__EntryPoint>>
function main() :mixed{
  $d = new DateTimeZone("America/New_York");
  var_dump($d->getLocation());
  $invalid_date = "2OO9-02--27 10:00?00.5";
  date_parse($invalid_date);
  var_dump(DateTime::getLastErrors());
  $abbrvs = DateTimeZone::listAbbreviations();
  invariant(is_darray($abbrvs), 'Expect root abbreviations to be dictlike');
  $yst = $abbrvs['yst'];
  invariant(is_varray($yst), 'Expect abbreviation array to be veclike');
  foreach ($yst as $abv) {
    invariant(is_darray($abv), 'Expect abbreviation to be dictlike');
  }
}
