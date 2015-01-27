<?php

ob_start();

for ($i = 0; $i < 10; $i++) {
  echo str_repeat('x', 0x7ffffffe >> 2);
}

ob_end_clean();
echo "Expected a fatal\n";
