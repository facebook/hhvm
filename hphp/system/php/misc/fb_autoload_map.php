<?php

function fb_autoload_map(mixed $map, string $root): bool {
  trigger_error(
    __FUNCTION__.'(): Use HH\autoload_set_paths() instead.',
    E_DEPRECATED
  );
  return HH\autoload_set_paths($map, $root);
}
