<?php

function foo($a)
{
   $a=5;
   try { echo func_get_arg(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
   try { echo func_get_arg(2,2); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
   try { echo func_get_arg("hello"); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
   echo func_get_arg(-1);
   echo func_get_arg(2);
}
foo(2);
echo "\n";
?>
