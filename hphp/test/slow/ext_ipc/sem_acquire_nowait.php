<?php

$sem = sem_get(uniqid());

var_dump(sem_acquire($sem));
var_dump(sem_acquire($sem, true));
