<?hh

<<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');
/* This will go wrong, once in a million times */
$ms = date_create()->format('u');
echo ($ms = 0) ? "microseconds = false\n" : "microseconds = true\n";

/* Normal creation */
echo date_create( "2016-10-03 12:47:18.819313" )->format( "Y-m-d H:i:s.u" ), "\n\n";

/* With modifications */
$dt = new DateTimeImmutable( "2016-10-03 12:47:18.819210" );
echo $dt->modify( "+1 day" )->format( "Y-m-d H:i:s.u" ), "\n";

$dt = new DateTimeImmutable( "2016-10-03 12:47:18.081921" );
echo $dt->modify( "-3 months" )->format( "Y-m-d H:i:s.u" ), "\n";

echo "\n";

/* These should reset the time (and hence fraction) to 0 */
$dt = new DateTimeImmutable( "2016-10-03 12:47:18.081921" );
echo $dt->modify( "yesterday" )->format( "Y-m-d H:i:s.u" ), "\n";

$dt = new DateTimeImmutable( "2016-10-03 12:47:18.081921" );
echo $dt->modify( "noon" )->format( "Y-m-d H:i:s.u" ), "\n";

$dt = new DateTimeImmutable( "2016-10-03 12:47:18.081921" );
echo $dt->modify( "10 weekday" )->format( "Y-m-d H:i:s.u" ), "\n";

/* Interval containing fractions */

$dt1 = new DateTimeImmutable( "2016-10-03 13:20:07.103123" );
$dt2 = new DateTimeImmutable( "2016-10-03 13:20:07.481312" );
$diff = $dt1->diff( $dt2 );

$dt0 = $dt1->sub( $diff );
$dt3 = $dt2->add( $diff );
$dt4 = $dt3->add( $diff );

echo $dt0->format( "Y-m-d H:i:s.u" ), "\n";
echo $dt1->format( "Y-m-d H:i:s.u" ), "\n";
echo $dt2->format( "Y-m-d H:i:s.u" ), "\n";
echo $dt3->format( "Y-m-d H:i:s.u" ), "\n";
echo $dt4->format( "Y-m-d H:i:s.u" ), "\n";
}
