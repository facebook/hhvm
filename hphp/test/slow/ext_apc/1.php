<?php

if (ini_get("apc.enabled") === "1") {
  echo "ok\n";
}
if (ini_get("apc.enable_cli") === "1") {
  echo "ok\n";
}
