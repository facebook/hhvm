<?php
openlog();
openlog(NULL, 'string', 0);

syslog();
syslog('Wrong parameter order', LOG_WARNING);

closelog('Doesnt take any parameters');
?>