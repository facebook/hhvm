<?php

session_set_cookie_params(10240, "ppp", "ddd", true, true);
var_dump(session_get_cookie_params());
var_dump(session_name("name1"));
var_dump(session_name("name2"));

session_start();
session_destroy();
