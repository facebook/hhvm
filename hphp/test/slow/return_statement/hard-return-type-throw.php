<?hh

function error_handler() {
  throw new Exception('nooo');
}

class Media {}

function crash(): string {
  $medias = varray[new Media];
  foreach ($medias as $media) {
    echo "about to return\n";
    return $media;
  }
}
<<__EntryPoint>>
function entrypoint_hardreturntypethrow(): void {
  set_error_handler(fun('error_handler'));

  crash();
}
