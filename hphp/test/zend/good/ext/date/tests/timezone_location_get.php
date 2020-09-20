<?hh <<__EntryPoint>> function main(): void {
$location = timezone_location_get(new DateTimeZone("Europe/Oslo"));
var_dump($location);
}
