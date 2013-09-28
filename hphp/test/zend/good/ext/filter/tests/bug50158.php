<?php

$email_address = "test=mail@example.com";    
var_dump(filter_var($email_address, FILTER_VALIDATE_EMAIL));    
                                                                
$email_address = "test-mail@example.com";    
var_dump(filter_var($email_address, FILTER_VALIDATE_EMAIL));    
    
$email_address = "test+mail@example.com";    
var_dump(filter_var($email_address, FILTER_VALIDATE_EMAIL));   

$email_address = "test?mail@example.com";    
var_dump(filter_var($email_address, FILTER_VALIDATE_EMAIL));   

?>