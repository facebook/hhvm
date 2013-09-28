<?php

$message = 'Der ist ein Süßwasserpool Süsswasserpool ... verschiedene Wassersportmöglichkeiten bei ...';

$pattern = '/\bwasser/iu';
preg_match_all($pattern, $message, $match, PREG_OFFSET_CAPTURE);
var_dump($match);

$pattern = '/[^\w]wasser/iu';
preg_match_all($pattern, $message, $match, PREG_OFFSET_CAPTURE);
var_dump($match);

?>