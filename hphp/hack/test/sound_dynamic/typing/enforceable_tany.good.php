<?hh

type FAKE<T> = T;

/* HH_FIXME[4101] Sadly, yes */
type MISSING = FAKE;

<<__SupportDynamicType>>
class C {
  public static MISSING $x = 3;
}
