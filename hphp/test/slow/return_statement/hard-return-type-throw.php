<?hh

function error_handler() :mixed{
  throw new Exception('nooo');
}

class Media {}

function crash(): string {
  $medias = vec[new Media];
  foreach ($medias as $media) {
    echo "about to return\n";
    return $media;
  }
}
<<__EntryPoint>>
function entrypoint_hardreturntypethrow(): void {
  set_error_handler(error_handler<>);

  crash();
}
