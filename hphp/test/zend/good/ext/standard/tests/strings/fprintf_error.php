<?hh

/* Testing Error Conditions */
<<__EntryPoint>> function main(): void {
echo "*** Testing Error Conditions ***\n";
/* zero argument */
try { var_dump( fprintf() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* scalar argument */
try { var_dump( fprintf(3) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* NULL argument */
try { var_dump( fprintf(NULL) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
