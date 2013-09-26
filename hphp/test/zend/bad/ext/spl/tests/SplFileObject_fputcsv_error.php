<?php
$fo = new SplFileObject(__DIR__ . '/SplFileObject_fputcsv_error.csv', 'w');

echo "*** Testing error conditions ***\n";
// zero argument
echo "-- Testing fputcsv() with zero argument --\n";
var_dump( $fo->fputcsv() );

// more than expected no. of args
echo "-- Testing fputcsv() with more than expected number of arguments --\n";
$fields = array("fld1", "fld2");
$delim = ";";
$enclosure ="\"";
var_dump( $fo->fputcsv($fields, $delim, $enclosure, $fo) );

echo "Done\n";?>
<?php
$file = __DIR__ . '/SplFileObject_fputcsv_error.csv';
unlink($file);
?>