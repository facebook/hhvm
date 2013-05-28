<?php

$standards = UConverter::getStandards();
var_dump(in_array('IANA', $standards));
var_dump(in_array('MIME', $standards));
