<?hh // partial

abstract final class Foo {}

/* HH_FIXME[4110] */
function f(): classname<Foo> {
}
