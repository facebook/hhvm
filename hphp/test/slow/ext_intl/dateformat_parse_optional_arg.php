<?php
$df = new IntlDateFormatter(Locale::getDefault(), 2, -1,
                            "America/Los_Angeles", 1, "MM*yyyy*dd");
$df->setLenient(false);
$timestamp = $df->parse("06*2010*02");
var_dump($timestamp);
