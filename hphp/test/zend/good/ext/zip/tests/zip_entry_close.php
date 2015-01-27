<?php
$zip    = zip_open(dirname(__FILE__)."/test_procedural.zip");
$entry  = zip_read($zip);
echo "entry_open:  "; var_dump(zip_entry_open($zip, $entry, "r"));
echo "entry_close: "; var_dump(zip_entry_close($entry));
echo "entry_close: "; var_dump(zip_entry_close($entry));
zip_close($zip);
?>
Done
