<?hh

class DateTimeZoneExt extends DateTimeZone
{
	public function __toString()
:mixed	{
		return parent::getName();
	}
}
<<__EntryPoint>>
function main_entry(): void {

  //Set the default time zone 
  date_default_timezone_set("Europe/London");

  echo "*** Testing basic DateTimeZone inheritance() ***\n";

  echo "\n-- Create an instance of DateTimeZoneExt --\n";
  $d = new DateTimeZoneExt("America/Los_Angeles");

  echo "\n-- Invoke __toString --\n";
  echo $d . "\n";

  echo "===DONE===\n";
}
