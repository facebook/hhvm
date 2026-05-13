<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

function baz():void {
  $vecpairs = vec[shape('a' => 2,'b' => "a"), shape('a' => 4,'b' => "b")];
  $dictpairs = dict[2 => shape('a' => "a", 'b' => 5), 3 => shape('a' => "b", 'b' => 7)];
  foreach ($vecpairs as shape('a' => $pi, 'b' => $a2)) {
    echo $pi;
    echo $a2;
  }

  foreach ($dictpairs as $i => shape('a' => $sa0, 'b' => $pi)) {
    echo $i;
    echo $sa0;
    echo $pi;
  }
}

<<__EntryPoint>>
function main_foreach_as_list() :mixed{
baz();
}
