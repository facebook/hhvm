<?php

TRY {
  ECHO "TRYING\n";
  Throw NEW Exception('LOL');
  ECHO "DON'T PRINT ME BRO\n";
} Catch (Exception $_) {
  ECHO "CATCHING\n";
} FINALLY {
  ECHO "WE'RE DONE HERE\n";
}
