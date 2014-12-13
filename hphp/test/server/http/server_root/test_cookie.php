<?php
setcookie($_GET['cookie_name'], $_GET['cookie_value'], 0, $_GET['cookie_path']);
var_dump($_GET['cookie_name']);
