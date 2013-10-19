<?php

echo "Before error, there should be an 'After error'\n";
@eval("invalid");
echo "After error\n";
