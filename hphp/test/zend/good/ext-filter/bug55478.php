<?php
$email_address = "test@xn--example--7za.de"; // "example-ä.de"
var_dump(filter_var($email_address, FILTER_VALIDATE_EMAIL));   
?>