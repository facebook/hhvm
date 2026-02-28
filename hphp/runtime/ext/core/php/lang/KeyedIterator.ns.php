<?hh

namespace HH {

interface KeyedIterator<+Tk, +Tv> extends Iterator<Tv>, KeyedTraversable<Tk, Tv> {
  // TODO(T121800572) We should add in the methods defined in the version of
  // this class in `hphp/hack/hhi/interfaces.hhi`
}

}
