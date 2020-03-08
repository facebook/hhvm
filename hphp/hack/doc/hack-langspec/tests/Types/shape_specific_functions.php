<?hh // strict

namespace NS_ShapeSpecificFunctions;

class C {
  const string KEYX = 'x';
  const string KEYY = 'y';
  const int KEYINTX = 10;
  const int KEYINTY = 23;
  const int KEYINTZ = 50;
}

function main(): void {
  echo "======== Shapes::idx ========\n";

  $s = shape('x' => 10, 'y' => 20);

  $v = Shapes::idx($s, 'x');		// field exists, return 10
  echo "\$v = " . (($v == null)? "null" : $v) ."\n";

  $v = Shapes::idx($s, 'y');		// field exists, return 20
  echo "\$v = " . (($v == null)? "null" : $v) ."\n";

  $v = Shapes::idx($s, 'z');		// field does not exist; return implict default, null
  echo "\$v = " . (($v == null)? "null" : $v) ."\n";

  $v = Shapes::idx($s, 'z', -99);	// field does not exist; return explicit default, -99
  echo "\$v = " . (($v == null)? "null" : $v) ."\n";

  echo "----------------------------\n";

  $s = shape(C::KEYINTX => 10, C::KEYINTY => 20);

  $v = Shapes::idx($s, C::KEYINTX);		// field exists, return 10
  echo "\$v = " . (($v == null)? "null" : $v) ."\n";

  $v = Shapes::idx($s, C::KEYINTY);	// field exists, return 20
  echo "\$v = " . (($v == null)? "null" : $v) ."\n";

  $v = Shapes::idx($s, C::KEYINTZ);	// field does not exist; return implict default, null
  echo "\$v = " . (($v == null)? "null" : $v) ."\n";

  $v = Shapes::idx($s, C::KEYINTZ, -99); // field does not exist; return explicit default, -99
  echo "\$v = " . (($v == null)? "null" : $v) ."\n";

  echo "\n======== Shapes::keyExists ========\n\n";

  $s = shape('id' => "23456", 'url' => "www.example.com", 'count' => 23);

  $v = Shapes::keyExists($s, 'url');		// field exists, return true
  echo "keyExists(\$s, 'x') = " . $v ."\n";

  $v = Shapes::keyExists($s, 'name');		// does not exist, return false
  echo "keyExists(\$s, 'name') = " . $v ."\n";

  echo "\n======== Shapes::removeKey ========\n\n";

  $s = shape();
  var_dump($s);
  Shapes::removeKey($s, 'name');	// no such field, so request ignored
  $a = Shapes::toArray($s);
  echo "# elements in array = " . count($a) . "\n";
  var_dump($s, $a);
  echo "----------------------------\n";

  $s = shape('x' => 10, 'y' => 20);
  var_dump($s);
  Shapes::removeKey($s, C::KEYX);	// field 'x' removed
  $a = Shapes::toArray($s);
  echo "# elements in array = " . count($a) . "\n";
  var_dump($s, $a);
  echo "----------------------------\n";

  $s = shape('id' => "23456", 'url' => "www.example.com", 'count' => 23);
  var_dump($s);
  Shapes::removeKey($s, 'url');		// field 'url' removed
  $a = Shapes::toArray($s);
  echo "# elements in array = " . count($a) . "\n";
  var_dump($s, $a);

  echo "\n======== Shapes::toArray ========\n\n";

  $s = shape();
  $a = Shapes::toArray($s);   // returns an array of 0 elements
  echo "# elements in array = " . count($a) . "\n";
  var_dump($s, $a);
  echo "----------------------------\n";

  $s = shape('x' => 10, 'y' => 20);
  $a = Shapes::toArray($s);
  echo "# elements in array = " . count($a) . "\n";
  var_dump($s, $a);
  echo "----------------------------\n";

  $s = shape('y' => 20, 'x' => 10);
  $a = Shapes::toArray($s);
  echo "# elements in array = " . count($a) . "\n";
  var_dump($s, $a);
  echo "----------------------------\n";

  $s = shape('id' => "23456", 'url' => "www.example.com", 'count' => 23);
  $a = Shapes::toArray($s);   // returns an array of 3 elements, of type string, string, and int, respectively
  echo "# elements in array = " . count($a) . "\n";
  var_dump($s, $a);
}

/* HH_FIXME[1002] call to main in strict*/
main();
