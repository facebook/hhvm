<?hh

function error_handler() {
  throw new Exception('nooo');
}
set_error_handler('error_handler');

class Media {}

function crash(): string {
  $medias = [new Media];
  foreach ($medias as $media) {
    echo "about to return\n";
    return $media;
  }
}

crash();
