<?hh <<__EntryPoint>> function main(): void {
for ($unix = 1461283200; $unix <= 1461369600; $unix += 8000) {
    echo "Time:", gmdate('Y-m-d H:i:s = B', $unix), PHP_EOL;
    echo "Time:", gmdate('Y-m-d H:i:s = B', $unix - 82 * 365 * 24 * 3600), PHP_EOL;
}
}
