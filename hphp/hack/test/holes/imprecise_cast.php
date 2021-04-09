<?hh

class Super {}

class Sub extends Super {}

function imprecise_cast(): num {
  $b = new Sub();
  /* HH_FIXME[4417] */
  return unsafe_cast<Super,int>($b);
}
