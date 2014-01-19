<?php

$f = fopen('php://stdout', 'w');
fprintf($f, 'stdout');
