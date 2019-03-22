<?php

try { var_dump(convert_cyr_string()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(convert_cyr_string("")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(convert_cyr_string("", "")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(convert_cyr_string("", "", ""));
try { var_dump(convert_cyr_string(array(), array(), array())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(convert_cyr_string((binary)"[[[[[[", "q", "m"));
var_dump(convert_cyr_string((binary)"[[[[[[", "k", "w"));
var_dump(convert_cyr_string((binary)"[[[[[[", "m", "a"));
var_dump(convert_cyr_string((binary)"[[[[[[", "d", "i"));
var_dump(convert_cyr_string((binary)"[[[[[[", "w", "k"));
var_dump(convert_cyr_string((binary)"[[[[[[", "i", "q"));
var_dump(convert_cyr_string((binary)"", "d", "i"));

echo "Done\n";
