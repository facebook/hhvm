<?hh
<<__EntryPoint>> function main(): void {
echo "\n*** Testing error conditions ***\n";
/* zero argument */
try { var_dump( addcslashes() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* unexpected arguments */
try { var_dump( addcslashes("foo[]") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( addcslashes('foo[]', "o", "foo") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n"; 
}
