<?php
fopen("php://fd", "w");
fopen("php://fd/", "w");
fopen("php://fd/-2", "w");
fopen("php://fd/1/", "w");

echo "\nDone.\n";