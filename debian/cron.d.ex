#
# Regular cron jobs for the hiphop-php package
#
0 4	* * *	root	[ -x /usr/bin/hiphop-php_maintenance ] && /usr/bin/hiphop-php_maintenance
