<?hh

<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function allowed()[\HH\Capabilities\IO]: void {
  echo("good");
  echo "good";
}

function disallowed0()[]: void {
  echo("bad - pure context");
  echo "bad - pure context";
}

function disallowed2()[non_det]: void {
  echo 42; // bad: non_det does not include IO
}
