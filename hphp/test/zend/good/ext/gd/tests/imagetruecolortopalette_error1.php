<?php
$resource = tmpfile();

imagetruecolortopalette($resource, true, 2);
try { imagetruecolortopalette('string', true, 2); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagetruecolortopalette(array(), true, 2); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { imagetruecolortopalette(null, true, 2); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
?>
