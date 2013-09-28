<?php

try {
	var_dump(new DatePeriod(NULL));
} catch (Exception $e) {
	var_dump($e->getMessage());
}

?>