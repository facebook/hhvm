<?php
session_start();
session_regenerate_id();
session_destroy();
echo "Done\n";
?>