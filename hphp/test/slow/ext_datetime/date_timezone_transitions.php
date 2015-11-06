<?php

$timezone = new DateTimeZone("Europe/London");
$transitions = $timezone->getTransitions(1000000, 999999999);
var_dump($transitions[0]);
var_dump($transitions[3]);
var_dump($transitions[count($transitions)-1]);


$timezone = new DateTimeZone('Asia/Kolkata');
// test all transitions before specified time range
$transitions = $timezone->getTransitions(1383176624, 1509407139);
var_dump($transitions);

// test all transitions when PHP_INT_MIN is supplied
// and an end before the first transition
$transitions = $timezone->getTransitions(-2147483648, -2047483648);
var_dump($transitions);

// test all transitions when PHP_INT_MIN is supplied
// and an end just after the first transition
$transitions = $timezone->getTransitions(-2147483648, 891582801);
var_dump($transitions);

// test start and end of range equal to transition time
$transitions = $timezone->getTransitions(-872058600, -862637400);
var_dump($transitions);

// test start of range equal to transition time
$transitions = $timezone->getTransitions(-872058600, -862637399);
var_dump($transitions);

// test end of range equal to transition time
$transitions = $timezone->getTransitions(-872058599, -862637400);
var_dump($transitions);

// test begin and end of range equal to same transition time
$transitions = $timezone->getTransitions(-862637400, -862637400);
var_dump($transitions);

// test begin and end of range equal
$transitions = $timezone->getTransitions(-862637399, -862637399);
var_dump($transitions);

// test begin after end of range equal
$transitions = $timezone->getTransitions(-862637399, -862637400);
var_dump($transitions);

$timezone = new DateTimeZone("UTC");
$transitions = $timezone->getTransitions(1000000, 999999999);
var_dump($transitions);
$transitions = $timezone->getTransitions();
var_dump($transitions);
