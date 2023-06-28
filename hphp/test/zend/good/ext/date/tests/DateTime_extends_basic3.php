<?hh

class DateTimeExt extends DateTime
{
	public function format($format = "F j, Y, g:i:s a")
:mixed	{
		return parent::format($format);
	}
}
<<__EntryPoint>>
function main_entry(): void {
  //Set the default time zone 
  date_default_timezone_set("Europe/London");

  echo "*** Testing new DateTime() : with user format() method ***\n";

  $d = new DateTimeExt("1967-05-01 22:30:41");
  echo $d->format() . "\n";

  echo "===DONE===\n";
}
