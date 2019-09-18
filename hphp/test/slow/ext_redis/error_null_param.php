<?hh
require __DIR__ . '/redis.inc';
<<__EntryPoint>> function main(): void {
NewRedisTestInstance()->publish(null, null);
echo "No Fatal";
}
