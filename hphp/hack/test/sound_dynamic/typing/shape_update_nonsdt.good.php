<?hh

class NonSDT { }

function expectDyn(dynamic $_):void { }

function test1(supportdyn<shape('a' => int, ...)> $s):void {
  expectDyn($s);
  // Update just the 'a' field with non-sd type
  $s['a'] = new NonSDT();
  // So 'b' field should support dynamic
  expectDyn(Shapes::idx($s, 'b'));
  // Now restore 'a' field
  $s['a'] = 3;
  expectDyn($s);
}
