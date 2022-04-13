<?hh <<__EntryPoint>> function main(): void {
$nano = time_nanosleep(2, 100000);

if ($nano === true) {
    echo "Slept for 2 seconds, 100 milliseconds.\n";
} else if ($nano === false) {
    echo "Sleeping failed.\n";
} else if (is_array($nano)) {
    $seconds = $nano['seconds'];
    $nanoseconds = $nano['nanoseconds'];
    echo "Interrupted by a signal.\n";
    echo "Time remaining: $seconds seconds, $nanoseconds nanoseconds.";
}
}
