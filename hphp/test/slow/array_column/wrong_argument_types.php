<?hh


<<__EntryPoint>>
function main_wrong_argument_types() :mixed{
try { array_column(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { array_column(varray[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { array_column('string'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { array_column(10); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
array_column(varray[], true);
array_column(varray[], varray[]);
array_column(varray[], 'correct', true);
array_column(varray[], 'correct', varray[]);
}
