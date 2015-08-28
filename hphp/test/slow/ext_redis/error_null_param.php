<?php
require __DIR__ . '/redis.inc';
NewRedisTestInstance()->publish(null, null);

echo "No Fatal";
