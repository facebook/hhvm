<?php


<<__EntryPoint>>
function main_old_ends() {
DECLARE (success=true) :
  ECHO "I DO DECLARE\n";
ENDDECLARE;

FOR ($i = 0; $i < 2; $i++) :
  ECHO "FOR $i\n";
ENDFOR;

FOREACH (ARRAY("one", "two") AS $n) :
  ECHO "FOREACH $i\n";
ENDFOREACH;

IF (TRUE) :
  ECHO "IN IF\n";
ELSE :
  ECHO "DON'T PRINT ME BRO\n";
ENDIF;

SWITCH (1) :
  CASE 0: ECHO "DON'T PRINT ME BRO\n"; BREAK;
  CASE 1: ECHO "IN SWITCH\n"; BREAK;
  DEFAULT: ECHO "DON'T PRINT ME BRO\n";
ENDSWITCH;

WHILE (TRUE) :
  ECHO "IN WHILE\n";
  BREAK;
  ECHO "DON'T PRINT ME BRO\n";
ENDWHILE;

ECHO "DONE\n";
}
