<?php

try { var_dump(constant()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(constant("", "")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(constant(""));

try { var_dump(constant(array())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

const TEST_CONST = 1;
var_dump(constant("TEST_CONST"));

const TEST_CONST2 = "test";
var_dump(constant("TEST_CONST2"));

echo "Done\n";
