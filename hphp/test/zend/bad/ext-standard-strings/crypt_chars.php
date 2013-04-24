<?php
var_dump(crypt("À1234abcd", "99"));
var_dump(crypt("À9234abcd", "99"));
var_dump(crypt("À1234abcd", "_01234567"));
var_dump(crypt("À9234abcd", "_01234567"));