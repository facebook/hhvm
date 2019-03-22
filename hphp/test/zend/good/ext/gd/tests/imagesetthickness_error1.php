<?php
$resource = tmpfile();

try { imagesetthickness('string', 5); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagesetthickness(array(), 5); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
imagesetthickness($resource, 5);
