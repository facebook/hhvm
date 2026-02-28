<?hh

<<__EntryPoint>>
function main_timelib_offset_at_time() :mixed{
$timeZone = new DateTimeZone('America/Guayaquil');
$dateTime = \DateTime::createFromFormat('U', '2371195560', $timeZone);
$var = $timeZone->getOffset($dateTime);
var_dump($var);
}
