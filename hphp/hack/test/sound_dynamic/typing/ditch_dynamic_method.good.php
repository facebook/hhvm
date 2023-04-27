<?hh

class NotSDT {}

<<__SupportDynamicType>>
class SDT {}

function f(): void {
  $x = "dyn_func";

  /* HH_FIXME[2121] */
  $y_fatal = NotSDT::$x(); // DITCH will make this fatal
  hh_expect_equivalent<dynamic>($y_fatal); // so it's safe to say this is dynamic (nothing)

  /* HH_FIXME[2121] */
  $y_ok = SDT::$x(); // SDT class guarantees the "dyn_func" method will return a value that supports dynamic
  hh_expect_equivalent<dynamic>($y_ok);
}
