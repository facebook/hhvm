<?php
function crash()
{
    $notDefined[$i] = 'test';
}

function error_handler() { return false; }

set_error_handler('error_handler');
crash();
echo "made it once\n";
crash();
echo "ok\n";
