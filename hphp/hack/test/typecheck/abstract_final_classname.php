<?hh // partial

abstract final class Foo {}

/* HH_FIXME[4336] */
function f(): classname<Foo> {
}
