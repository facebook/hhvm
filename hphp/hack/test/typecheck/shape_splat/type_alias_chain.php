<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Multi-level type alias chain with splats
type Base = shape('id' => int);
type WithName = shape(...Base, 'name' => string);
type WithEmail = shape(...WithName, 'email' => string);
type WithAge = shape(...WithEmail, ?'age' => int);

function test_chain(WithAge $s): void {
  hh_expect<int>($s['id']);
  hh_expect<string>($s['name']);
  hh_expect<string>($s['email']);
  hh_expect<shape(?'age' => int, 'email' => string, 'id' => int, 'name' => string)>($s);
}
