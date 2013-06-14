<?php
apache_note("blarb", "foo");
if (apache_note("blarb", "smurf") === "foo") {
  echo "ok\n";
}
if (apache_note("blarb") === "smurf") {
  echo "ok\n";
}
