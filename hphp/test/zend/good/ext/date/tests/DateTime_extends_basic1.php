<?hh


class DateTimeExt extends DateTime
{
	public static $format = "F j, Y, g:i:s a";
	
	public function __toString()
:mixed	{
		return parent::format(self::$format);
	}
}
<<__EntryPoint>>
function main_entry(): void {

  //Set the default time zone 
  date_default_timezone_set("Europe/London");

  echo "*** Testing basic DateTime inheritance() ***\n";

  echo "\n-- Create an instance of DateTimeExt --\n";
  $d = new DateTimeExt("1967-05-01 22:30:41");

  echo "\n-- Invoke __toString --\n";
  echo $d . "\n";

  echo "\n -- modify date and time --\n";
  $d->setDate(1963, 7, 2); 
  $d->setTime(10, 45, 30); 

  echo "\n-- Invoke __toString again --\n";
  echo $d . "\n";

  echo "===DONE===\n";
}
