<?php <<__EntryPoint>> function main() {
$location = timezone_location_get(new DateTimeZone("Europe/Oslo"));
var_dump($location);
}
