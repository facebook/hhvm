<?php
ini_set("intl.error_level", E_WARNING);

var_dump(IntlTimeZone::createTimeZoneIDEnumeration());
var_dump(IntlTimeZone::createTimeZoneIDEnumeration(array()));
var_dump(IntlTimeZone::createTimeZoneIDEnumeration(-1));
var_dump(IntlTimeZone::createTimeZoneIDEnumeration(IntlTimeZone::TYPE_ANY, array()));
var_dump(IntlTimeZone::createTimeZoneIDEnumeration(IntlTimeZone::TYPE_ANY, "PT", "a80"));
