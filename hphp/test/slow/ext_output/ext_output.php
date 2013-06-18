<?php

function mytolower($a) {
  return strtolower($a);
}

// don't print things here, since this tests what accumulates in the
// output buffer.
function VS($x, $y) {
  if ($x !== $y) {
    throw new Exception("test failed: got $x, expected $y");
  }
}

//////////////////////////////////////////////////////////////////////

ob_start();
ob_start("mytolower");
echo "TEst";
ob_end_flush();
VS(ob_get_clean(), "test");

ob_start();
echo "test";
ob_clean();
VS(ob_get_clean(), "");

ob_start();
ob_start("mytolower");
echo "TEst";
ob_flush();
VS(ob_get_clean(), "");
VS(ob_get_clean(), "test");

ob_start();
ob_start("mytolower");
echo "TEst";
ob_end_clean();
VS(ob_get_clean(), "");

ob_start();
ob_start("mytolower");
echo "TEst";
ob_end_flush();
VS(ob_get_clean(), "test");

ob_start();
ob_start();
echo "test";
VS(ob_get_clean(), "test");
VS(ob_get_clean(), "");
VS(ob_get_clean(), "");

ob_start();
echo "test";
VS(ob_get_contents(), "test");
VS(ob_get_contents(), "test"); // verifying content stays
ob_end_clean();

ob_start();
ob_start();
echo "test";
VS(ob_get_flush(), "test");
VS(ob_get_flush(), "");
ob_end_clean();
VS(ob_get_flush(), "test");
ob_end_clean();

ob_start();
echo "test";
VS(ob_get_length(), 4);
ob_end_clean();

VS(ob_get_level(), 0);
ob_start();
VS(ob_get_level(), 1);
ob_end_clean();
VS(ob_get_level(), 0);

ob_get_status();

ob_start();
ob_start("test");
$handlers = ob_list_handlers();
ob_end_clean();
ob_end_clean();
VS($handlers, array(null, "test"));

echo "\nok\n";
