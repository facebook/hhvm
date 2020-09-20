<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('Asia/Jerusalem');

$t = mktime(0,0,0, 6, 27, 2006);

try { var_dump(strftime()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(strftime(""));
var_dump(strftime("%a %A %b %B %c %C %d %D %e %g %G %h %H %I %j %m %M %n %p %r %R %S %t %T %u %U %V %W %w %x %X %y %Y %Z %z %%", $t));
var_dump(strftime("%%q %%a", $t));
var_dump(strftime("%q", $t));
var_dump(strftime("blah", $t));

try { var_dump(gmstrftime()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(gmstrftime(""));
var_dump(gmstrftime("%a %A %b %B %c %C %d %D %e %g %G %h %H %I %j %m %M %n %p %r %R %S %t %T %u %U %V %W %w %x %X %y %Y %Z %z %%", $t));
var_dump(gmstrftime("%%q %%a", $t));
var_dump(gmstrftime("%q", $t));
var_dump(gmstrftime("blah", $t));

echo "Done\n";
}
