<?php

$çity  = "San Francisco";
$state = "CA";
$event = "SIGGRAPH";

$location_vars = array("c\u0327ity", "state");

$result = compact("event", $location_vars);
var_dump($result);
?>