<?hh

<<__EntryPoint>> function main(): void {
  ob_start(function($str) {
    throw new Exception('chickpeas');
    return $str.'!!!';
  });

  echo 'garbanzo beans';

  ob_end_flush();

  echo "DON'T PRINT ME!";
}
