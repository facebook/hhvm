<?hh
<<__EntryPoint>> function main(): void {
var_dump(gmp_init("98765678"));
var_dump(gmp_strval(gmp_init("98765678")));
try { var_dump(gmp_strval(gmp_init())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_init()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gmp_init(1,2,3,4)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(gmp_init(1,-1));
var_dump(gmp_init("",36));
var_dump(gmp_init("foo",3));
var_dump(gmp_strval(gmp_init("993247326237679187178",3)));

echo "Done\n";
}
