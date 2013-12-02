<?php

$subject = 'done';
for ($i = 0; $i < 97 * 1024; $i++) {
	preg_replace('/(' . $i . ')/ui', 'replaced', $subject);
}

var_dump($subject);
