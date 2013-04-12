<?php

 var_dump(filetype('test/test_ext_file2.tmp'));var_dump(is_link('test/test_ext_file2.tmp'));$a = lstat('test/test_ext_file2.tmp');var_dump($a['mtime']);