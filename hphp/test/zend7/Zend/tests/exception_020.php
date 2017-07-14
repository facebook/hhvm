<?php

class MyErrorException extends ErrorException{}
throw new MyErrorException(new stdClass);

?>
