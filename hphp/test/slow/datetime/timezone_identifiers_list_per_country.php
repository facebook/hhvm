<?php

$country_codes = array(
  'US',
  'PT',
  'GR',
  'IL',
  'BZ',
);

foreach ($country_codes as $code) {
  var_dump(DateTimeZone::listIdentifiers(DateTimeZone::PER_COUNTRY, $code));
}
