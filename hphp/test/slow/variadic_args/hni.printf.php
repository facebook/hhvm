<?php

// Straight unpacking and repacking (or transferrence)
printf("Pi is 3.%d4%c5%s2\n", ...[1, ord('1'), 9]);

// Splat unpacked and repacked
printf(...["Roses are %s, Violets are %s\n", "Red", "Blue"]);

// Splat unpacking and repacking with new args
printf("The meaning of life is %d not %d, or %s\n", 43, ...[42, "Yellow"]);
