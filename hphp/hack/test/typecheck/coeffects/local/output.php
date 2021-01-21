<?hh

function allowed()[\HH\Capabilities\IO]: void {
  echo("good");
  echo "good";
}

function disallowed0()[]: void {
  echo("bad - pure context");
  echo "bad - pure context";
}

function disallowed2()[write_props]: void {
  echo 42; // bad: write_props does not include IO
}

function allowed_by_default(): void {
  echo("good");
  echo "good";
}
