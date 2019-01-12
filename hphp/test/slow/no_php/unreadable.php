<?hh

$f = tempnam(sys_get_temp_dir(), "phpincludetest");
file_put_contents($f, '<?hh echo "unreadable\n";');
chmod($f, 0266);
fb_call_user_func_async(__DIR__.'/unreadable_impl.inc', 'run', $f);
unlink($f);
