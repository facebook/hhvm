<?hh

class DateTimeExt1 extends DateTime {
	public function __clone() :mixed{
		echo "-- DateTimeExt1 __clone magic method called --\n"; 
	}
}
<<__EntryPoint>>
function main_entry(): void {
  //Set the default time zone 
  date_default_timezone_set("Europe/London");

  //Set the default time zone 
  date_default_timezone_set("Europe/London");

  echo "*** Testing clone of objects derived from DateTime class with __clone magic method***\n";

  $d1 = new DateTimeExt1("2009-02-03 12:34:41 GMT");
  $d1_clone = clone $d1;

  //verify clone by calling method on new object
  var_dump( $d1_clone->format( "m.d.y") ); 

  echo "===DONE===\n";
}
