<?hh


<<__EntryPoint>>
function main_timezone_identifiers_list_per_country() :mixed{
$country_codes = vec[
  'US',
  'PT',
  'GR',
  'IL',
  'BZ',
];

foreach ($country_codes as $code) {
  var_dump(DateTimeZone::listIdentifiers(DateTimeZone::PER_COUNTRY, $code));
}
}
