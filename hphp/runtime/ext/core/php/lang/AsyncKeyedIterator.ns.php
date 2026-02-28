<?hh

namespace HH {

interface AsyncKeyedIterator<+Tk, +Tv> extends AsyncIterator<Tv> {}
  // TODO(T121800572) We should add in the methods defined in the version of
  // this class in `hphp/hack/hhi/interfaces.hhi`
}
