/*
 *  Duktape 2.1.0 adds a 'global' binding.  Polyfill for earlier versions.
 */

if (typeof global === 'undefined') {
    (function () {
        var global = new Function('return this;')();
        Object.defineProperty(global, 'global', {
            value: global,
            writable: true,
            enumerable: false,
            configurable: true
        });
    })();
}
