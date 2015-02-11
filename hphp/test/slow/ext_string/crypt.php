<?php
/*
 * Raise Notice:
 * 5.6.0Raise E_NOTICE security warning if salt is omitted.
 */
var_dump(
  crypt('$2a$07$' . str_repeat('./', 11)),
  crypt('$2x$07$' . str_repeat('./', 11)),
  crypt('$2y$07$' . str_repeat('./', 11), '678')
);
