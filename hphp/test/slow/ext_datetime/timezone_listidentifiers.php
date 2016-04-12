<?php
echo "ALL (implicit):", PHP_EOL;
echo count(DateTimeZone::listIdentifiers());
echo PHP_EOL, "ALL (explicit):", PHP_EOL;
echo count(DateTimeZone::listIdentifiers(DateTimeZone::ALL));
echo PHP_EOL, "PER_COUNTRY:US:", PHP_EOL;
echo count(DateTimeZone::listIdentifiers(DateTimeZone::PER_COUNTRY, "US"));
echo PHP_EOL, "ALL_WITH_BC:", PHP_EOL;
echo count(DateTimeZone::listIdentifiers(DateTimeZone::ALL_WITH_BC));
echo PHP_EOL;
