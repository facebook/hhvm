<?hh

class Chickpea {
  public function __toString() :mixed{
    throw new Exception('chickpeas');
    return 'chickpea';
  }
}

<<__EntryPoint>> function main(): void {
  ob_start(function($str) {
    return new Chickpea();
  });

  echo 'garbanzo beans';

  ob_end_flush();

  echo "DON'T PRINT ME!";
}
