// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: type static::_CC = {
// CHECK:   vtable: **void
// CHECK: }

// CHECK: type _CC = {
// CHECK:   vtable: **void
// CHECK: }

// CHECK: global vtable::static::_CC
// CHECK: global vtable::_CC
// CHECK: global static_singleton::_CC

// CHECK: define get_static::_CC() : *static::_CC
// CHECK:   n0: *static::_CC = load &static_singleton::_CC
// CHECK:   n1 = raw_ptr_is_null(n0)
// CHECK:   jmp b2, b1
// CHECK:   #b2:
// CHECK:   prune ! n1
// CHECK:   ret n0
// CHECK:   #b1:
// CHECK:   prune n1
// CHECK:   n2 = alloc_words(3)
// CHECK:   store &vtable::static::_CC <- n2: **void
// CHECK:   store n2[0] <- &invoke::static::_CC: *void
// CHECK:   store n2[1] <- &get_prop::static::_CC: *void
// CHECK:   store n2[2] <- &set_prop::static::_CC: *void
// CHECK:   n3 = alloc_words(3)
// CHECK:   store &vtable::_CC <- n3: **void
// CHECK:   store n3[0] <- &invoke::_CC: *void
// CHECK:   store n3[1] <- &get_prop::_CC: *void
// CHECK:   store n3[2] <- &set_prop::_CC: *void
// CHECK:   n4 = alloc_words(1)
// CHECK:   store n4[0] <- n2: *void
// CHECK:   store &static_singleton::_CC <- n4: *static::_CC
// CHECK:   ret n4

// CHECK: define invoke::static::_CC(name: *Mixed, params: *HackParams) : *Mixed
// CHECK:   n0: *Mixed = load &name
// CHECK:   n1: *HackParams = load &params
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n2 = hack_bad_method_call()

// CHECK: define invoke::_CC(name: *Mixed, params: *HackParams) : *Mixed
// CHECK:   n0: *Mixed = load &name
// CHECK:   n1: *HackParams = load &params
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n2 = hack_bad_method_call()

// CHECK: define get_prop::static::_CC(name: *Mixed) : *Mixed
// CHECK:   n0: *Mixed = load &name
// CHECK:   n1 = hack_bad_property()

// CHECK: define get_prop::_CC(name: *Mixed) : *Mixed
// CHECK:   n0: *Mixed = load &name
// CHECK:   n1 = hack_bad_property()

// CHECK: define set_prop::static::_CC(name: *Mixed, value: *Mixed) : void
// CHECK:   n0: *Mixed = load &name
// CHECK:   n1: *Mixed = load &value
// CHECK:   n2 = hack_bad_property()

// CHECK: define set_prop::_CC(name: *Mixed, value: *Mixed) : void
// CHECK:   n0: *Mixed = load &name
// CHECK:   n1: *Mixed = load &value
// CHECK:   n2 = hack_bad_property()

class C {
}
