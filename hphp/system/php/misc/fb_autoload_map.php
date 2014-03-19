<?php

/**
 * Deprecated - use HH\autoload_set_paths instead.
 */
function fb_autoload_map(mixed $map, string $root): bool {
  return HH\autoload_set_paths($map, $root);
}
