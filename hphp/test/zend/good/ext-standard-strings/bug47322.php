<?php

sscanf(":59:58","%s:%d:%f", $a, $b, $c);
echo "[$a][$b][$c]\n";

sscanf("15:01:58.2","%d:%f:%f", $a, $b, $c);
echo "[$a][$b][$c]\n";

sscanf("15.1111::foo","%f:%d:%s", $a, $b, $c);
echo "[$a][$b][$c]\n";

?>