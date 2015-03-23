<?php
// Copyright 2004-present Facebook. All Rights Reserved.

$status = -123;
pcntl_wait($status, WNOHANG);
pcntl_waitpid(-1, $status, WNOHANG);
var_dump($status);
