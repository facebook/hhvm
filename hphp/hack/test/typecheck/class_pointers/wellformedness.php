<?hh

class G<T as string> {}
class C {}

function f(
  G<classname<C>> $g_cn,
  G<class<C>> $g_c,
  dict<classname<C>, class<C>> $d_cn_to_c,
  dict<class<C>, class<C>> $d_c_to_c,
): void {}
