<?hh
class TDate extends DateTime {
  public function format($format) :mixed{
    return "lol";
  }
}

<<__EntryPoint>>
function main_format_function_not_overridden() :mixed{
$date = "12/23/2008 13:45";
$tz = "US/Central";
$tdate = new TDate($date, new DateTimeZone($tz));
var_dump(date_format($tdate, 'D m/d/Y H:i'));
var_dump($tdate->format('D m/d/Y H:i', true));

$datetime = new DateTime($date, new DateTimeZone($tz));
}
