<?hh
/* Prototype  : public int DateTime::getOffset  ( void  )
 * Description: Returns the daylight saving time offset
 * Source code: ext/date/php_date.c
 * Alias to functions:  date_offset_get
 */

//Set the default time zone
<<__EntryPoint>> function main(): void {
date_default_timezone_set('Europe/London');
echo "*** Testing DateTime::getOffset() : basic functionality ***\n";

$winter = new DateTime('2008-12-25 14:25:41');
$summer = new DateTime('2008-07-02 14:25:41');

echo "Winter offset: " . $winter->getOffset() / 3600 . " hours\n";
echo "Summer offset: " . $summer->getOffset() / 3600 . " hours\n";

echo "===DONE===\n";
}
