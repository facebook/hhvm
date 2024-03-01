<?hh
<<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');
setlocale(LC_ALL, 'C');

print "TZ has NOT been set\n";
print "Should strftime==datestr? Strftime seems to assume GMT tStamp.\n";
$input = "10:00:00 AM July 1 2005";
print "input    " . $input . "\n";
$tStamp = strtotime($input);
print "strftime " . strftime("%r %B%e %Y %Z %z", $tStamp) . "\n";
print "datestr  " . date ("H:i:s A F j Y T", $tStamp) . "\n";

print "\nSetting TZ\n";
date_default_timezone_set('Australia/Sydney');
putenv("TZ=Australia/Sydney");
$input = "10:00:00 AM July 1 2005";
print "input    " . $input . "\n";
$tStamp = strtotime($input);
print "strftime " . strftime("%r %B%e %Y %Z %z", $tStamp) . "\n";
print "datestr  " . date ("H:i:s A F j Y T", $tStamp) . "\n";
}
