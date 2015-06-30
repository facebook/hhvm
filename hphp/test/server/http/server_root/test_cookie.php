<?php
setcookie($_GET['cookie_name'],
          $_GET['cookie_value'],
          0,
          $_GET['cookie_path'],
          $_GET['cookie_domain']);
var_dump($_GET['cookie_name']);
