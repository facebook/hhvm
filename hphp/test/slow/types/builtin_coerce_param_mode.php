<?hh
<<__EntryPoint>> function main(): void {
// HNI ParamCoerceModeNull
try { var_dump(bcscale(varray[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$x = 'bcscale';
try { var_dump($x(varray[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// IDL ParamCoerceModeNull
try { var_dump(sqrt(varray[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$x = 'sqrt';
try { var_dump($x(varray[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
