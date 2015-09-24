<?php
var_dump(chmod('', 0777));
var_dump(touch(''));
var_dump(disk_free_space(''));
var_dump(chmod(null, 0777));
var_dump(touch(null));
var_dump(disk_free_space(null));
var_dump(chmod(false, 0777));
var_dump(touch(false));
var_dump(disk_free_space(false));
