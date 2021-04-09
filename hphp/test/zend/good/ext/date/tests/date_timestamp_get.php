<?hh <<__EntryPoint>> function main(): void {
$beginDtObj = date_create('1970-01-01T00:00:00UTC');
$beginTimestamp = date_timestamp_get($beginDtObj);
var_dump($beginTimestamp);

// Test the DateTime feature alias in function date_timestamp_get().
$dateTimeTz = (new DateTime('1970-01-01T00:00:00UTC'))->getTimestamp();
var_dump($dateTimeTz === $beginTimestamp);
}
