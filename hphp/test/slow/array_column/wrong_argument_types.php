<?hh


<<__EntryPoint>>
function main_wrong_argument_types() :mixed{
try { array_column(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { array_column(vec[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { array_column('string'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { array_column(10); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
array_column(vec[], true);
array_column(vec[], vec[]);
array_column(vec[], 'correct', true);
array_column(vec[], 'correct', vec[]);
}
