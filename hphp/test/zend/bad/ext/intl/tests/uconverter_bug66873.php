<?php
    $o = new UConverter(1, 1);
    $o->toUCallback(1, 1, 1, $b);
    var_dump($o->getErrorCode());
