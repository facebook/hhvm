<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Bar{}
final class :xhp_ui extends XHPTest {

  attribute
    mixed myprop;
}
<<__EntryPoint>>
function test() : void {
  $y = readonly new Bar();
  $z = <xhp_ui myprop={$y} />;

}
