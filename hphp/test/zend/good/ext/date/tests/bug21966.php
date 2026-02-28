<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('Europe/London');
echo '27/3/04 = ' . strval(mktime(0,0,0,3,27,2004)) . "\n";   // 1080345600
echo '28/3/04 = ' . strval(mktime(0,0,0,3,28,2004)) . "\n";   // -3662  - should be 108043200
echo '28/3/04 = ' . strval(mktime(2,0,0,3,28,2004)) . "\n";   // 1080435600
echo '29/3/04 = ' . strval(mktime(0,0,0,3,29,2004)) . "\n";   // 1080514800
echo '30/3/04 = ' . strval(mktime(0,0,0,3,30,2004)) . "\n";   // 1080601200
}
