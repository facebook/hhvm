<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(dcngettext(1,1,1,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(dcngettext('1','1','1',1,1));
var_dump(dcngettext("test","test","test",1,1));
var_dump(dcngettext("test","test","test",0,0));
var_dump(dcngettext("test","test","test",-1,-1));
var_dump(dcngettext("","","",1,1));
var_dump(dcngettext("","","",0,0));

echo "Done\n";
}
