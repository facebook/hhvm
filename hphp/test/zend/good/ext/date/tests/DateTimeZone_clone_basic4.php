<?hh

class DateTimeZoneExt1 extends DateTimeZone {
	public function __clone() :mixed{
		echo "-- DateTimeExt1 __clone magic method called --\n"; 
	}
}
<<__EntryPoint>>
function main_entry(): void {
  //Set the default time zone 
  date_default_timezone_set("Europe/London");

  echo "*** Testing clone of objects derived from DateTimeZone class with __clone magic method***\n";

  $d1 = new DateTimeZoneExt1("America/New_York");
  $d1_clone = clone $d1;

  //verify clone by calling method on new object
  var_dump( $d1_clone->getName() ); 

  echo "===DONE===\n";
}
