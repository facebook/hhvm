<?php
ini_set('session.save_handler', 'redis');
ini_set('session.save_path',
        'tcp://127.0.0.1:6379?database=5&prefix=PHPREDIS_SESSION:');
session_start();
var_dump(session_destroy());
