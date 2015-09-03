<?php
echo "ALL:", PHP_EOL;
echo count(DateTimeZone::listIdentifiers(DateTimeZone::ALL));
echo PHP_EOL, "PER_COUNTRY:", PHP_EOL;
echo count(DateTimeZone::listIdentifiers(DateTimeZone::ALL_WITH_BC));
echo PHP_EOL, "ALL_WITH_BC:", PHP_EOL;
echo count(DateTimeZone::listIdentifiers(DateTimeZone::ALL_WITH_BC));
echo PHP_EOL;
