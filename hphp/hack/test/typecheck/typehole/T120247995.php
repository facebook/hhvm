<?hh

function gogogo(): void {
  $local = null;
  foreach(vec[1] as $value) {
    if ($local is null) {
      $local = $value;
      continue;
    }
    $local->ohno();
  }
}
