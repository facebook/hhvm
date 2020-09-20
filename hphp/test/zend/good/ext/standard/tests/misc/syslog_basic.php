<?hh <<__EntryPoint>> function main(): void {
openlog('phpt', LOG_NDELAY | LOG_PID, LOG_USER);

syslog(LOG_WARNING, 'Basic syslog test');

closelog();
echo "===DONE===\n";
}
