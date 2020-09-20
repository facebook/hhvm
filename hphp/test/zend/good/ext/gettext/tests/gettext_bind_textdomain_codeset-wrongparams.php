<?hh <<__EntryPoint>> function main(): void {
try { bind_textdomain_codeset('messages'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { bind_textdomain_codeset('messages','foo','bar'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
