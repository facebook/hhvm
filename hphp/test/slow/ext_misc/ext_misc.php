<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x, true); }

//////////////////////////////////////////////////////////////////////


VERIFY(connection_aborted() != true);
VERIFY(connection_status() == CONNECTION_NORMAL);
VERIFY(connection_timeout() != true);
constant("a");
VERIFY(defined("a") != true);
__halt_compiler();
ignore_user_abort("a");
VERIFY(empty(uniqid()) != true);
VS(token_name(258), "T_REQUIRE_ONCE");
VS(count(sys_getloadavg()), 3);

$src = "blarb <?php 1";
VS(token_get_all(src),
   array(array(T_INLINE_HTML, "blarb ", 1),
         array(T_OPEN_TAG, "<?php", 1)));
