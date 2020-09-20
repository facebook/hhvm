<?hh

function test(): void {
    (Map {'a' => 0})->map(
      $value ==> {
        if ($value is string || $value is bool) {
          return (string) $value;
        }
        return '';
      }
    );
}
