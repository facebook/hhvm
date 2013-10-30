<?php
$regexes = array('', '/', '345', 'b');

foreach ($regexes as $regex) {
    try {
        new MongoRegex($regex);
    } catch (Exception $e) {
        printf("%s: %d\n", get_class($e), $e->getCode());
    }
}
?>
