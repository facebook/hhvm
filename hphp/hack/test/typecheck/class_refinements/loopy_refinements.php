<?hh



abstract class MyUBase {
  abstract const type TV as MyVBase with { type TU as this; };
}

abstract class MyVBase {
  abstract const type TU as MyUBase with { type TV as this; };
}

/* These have to be final. */
final class MyU extends MyUBase {
  const type TV = MyV;
}

final class MyV extends MyVBase {
  const type TU = MyU;
}
