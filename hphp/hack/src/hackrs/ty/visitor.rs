// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl;
use crate::local;
use crate::reason::Reason;

/// A type which can be traversed by a `Visitor`.
pub trait Walkable<R: Reason> {
    fn accept(&self, v: &mut dyn Visitor<R>) {
        self.recurse(v);
    }
    fn recurse(&self, _v: &mut dyn Visitor<R>) {}
}

/// A visitor over data structures containing decls or types.
pub trait Visitor<R: Reason> {
    /// Must return `self`.
    fn object(&mut self) -> &mut dyn Visitor<R>;

    fn visit_pos(&mut self, _: &R::Pos) {}
    fn visit_symbol(&mut self, _: &pos::Symbol) {}

    fn visit_type_name(&mut self, o: &pos::TypeName) {
        o.recurse(self.object());
    }
    fn visit_module_name(&mut self, o: &pos::ModuleName) {
        o.recurse(self.object());
    }
    fn visit_const_name(&mut self, o: &pos::ConstName) {
        o.recurse(self.object());
    }
    fn visit_fun_name(&mut self, o: &pos::FunName) {
        o.recurse(self.object());
    }
    fn visit_class_const_name(&mut self, o: &pos::ClassConstName) {
        o.recurse(self.object());
    }
    fn visit_type_const_name(&mut self, o: &pos::TypeConstName) {
        o.recurse(self.object());
    }
    fn visit_method_name(&mut self, o: &pos::MethodName) {
        o.recurse(self.object());
    }
    fn visit_prop_name(&mut self, o: &pos::PropName) {
        o.recurse(self.object());
    }

    fn visit_decl_ty(&mut self, o: &decl::Ty<R>) {
        o.recurse(self.object());
    }
    fn visit_local_ty(&mut self, o: &local::Ty<R>) {
        o.recurse(self.object());
    }

    fn visit_decl(&mut self, o: &crate::decl::shallow::Decl<R>) {
        o.recurse(self.object());
    }
    fn visit_named_decl(&mut self, o: &crate::decl::shallow::NamedDecl<R>) {
        o.recurse(self.object());
    }
    fn visit_shallow_class(&mut self, o: &crate::decl::ShallowClass<R>) {
        o.recurse(self.object());
    }
}

impl<R: Reason, T: Walkable<R>> Walkable<R> for Option<T> {
    fn recurse(&self, v: &mut dyn Visitor<R>) {
        match self {
            Some(some) => some.accept(v),
            None => {}
        }
    }
}

impl<R: Reason, T: Walkable<R> + ?Sized> Walkable<R> for &T {
    fn recurse(&self, v: &mut dyn Visitor<R>) {
        let obj: &T = self;
        obj.accept(v)
    }
}

impl<R: Reason, T: Walkable<R> + ?Sized> Walkable<R> for Box<T> {
    fn recurse(&self, v: &mut dyn Visitor<R>) {
        let obj: &T = self;
        obj.accept(v)
    }
}

impl<R: Reason, T: Walkable<R>> Walkable<R> for [T] {
    fn recurse(&self, v: &mut dyn Visitor<R>) {
        for obj in self {
            obj.accept(v);
        }
    }
}

impl<R: Reason, T: Walkable<R>> Walkable<R> for Vec<T> {
    fn recurse(&self, v: &mut dyn Visitor<R>) {
        for obj in self {
            obj.accept(v);
        }
    }
}

impl<R: Reason, K: Walkable<R>, V: Walkable<R>> Walkable<R> for std::collections::BTreeMap<K, V> {
    fn recurse(&self, v: &mut dyn Visitor<R>) {
        for (key, val) in self {
            key.accept(v);
            val.accept(v);
        }
    }
}

impl<R: Reason, T: Walkable<R>> Walkable<R> for hcons::Hc<T> {
    fn recurse(&self, v: &mut dyn Visitor<R>) {
        let obj: &T = self;
        obj.accept(v)
    }
}

impl<R: Reason, S: Copy + Walkable<R>> Walkable<R> for pos::Positioned<S, R::Pos> {
    fn recurse(&self, v: &mut dyn Visitor<R>) {
        v.visit_pos(self.pos());
        self.id_ref().accept(v);
    }
}

/// Generate an impl of `Walkable<R>` for the given type which recurses on the
/// given fields.
///
/// # Examples
///
/// Suppose we have this struct definition:
///
///     struct Foo<R: Reason> {
///         pos: R::Pos,
///         ty: Ty<R>,
///         constraint: Ty<R>,
///     }
///
/// We can generate an impl of `Walkable<R>` for `Foo<R>` like this:
///
///     walkable!(Foo<R> => [ty, constraint]);
///
/// The macro will expand to something like the following:
///
///     impl<R: Reason> Walkable<R> for Foo<R> {
///         fn recurse(&self, v: &mut dyn Visitor<R>) {
///             self.ty.accept(v);
///             self.constraint.accept(v);
///         }
///     }
///
/// Note that the macro implicitly introduces the type parameter `R`.
///
/// If the type is one which a `Visitor` may be interested in handling, add a
/// `visit_` method to the `Visitor` trait, and reference that method with the
/// `as` keyword in the `walkable!` macro:
///
///     walkable!(Foo<R> as visit_foo => [ty, constraint]);
///
/// This will expand to:
///
///     impl<R: Reason> Walkable<R> for Foo<R> {
///         fn accept(&self, v: &mut dyn crate::visitor::Visitor<R>) {
///             v.visit_foo(self);
///         }
///         fn recurse(&self, v: &mut dyn Visitor<R>) {
///             self.ty.accept(v);
///             self.constraint.accept(v);
///         }
///     }
///
/// If the type has type parameters other than `R`:
///
///     struct Foo<R: Reason, T> {
///         pos: R::Pos,
///         ty: T,
///         constraint: T,
///     }
///
/// Use the `impl` and `for` keywords to introduce all type parameters. Note
/// that the `R: Reason` parameter is no longer implicitly introduced:
///
///     walkable!(impl<R: Reason, T> for Foo<R, T> as visit_foo => [ty, constraint]);
///
/// For enums:
///
///     enum Typeconst<R: Reason> {
///         Abstract(AbstractTypeconst<R>),
///         Concrete(ConcreteTypeconst<R>),
///     }
///
/// Write a list of `pattern => [fields]` arms in curly braces:
///
///     walkable!(Typeconst<R> as visit_typeconst => {
///         Self::Abstract(at) => [at],
///         Self::Concrete(ct) => [ct],
///     });
///
/// For leaves (structures which cannot contain the types we are interested in
/// visiting), either 1) don't implement `Walkable<R>`, and don't specify fields
/// of that type in implementations of `Walkable<R>` for other types (as done
/// with the field `pos` in `Foo<R>` in the example above), or 2) use
/// `walkable!` to generate a no-op implementation of `Walkable<R>` (when not
/// implementing `Walkable<R>` would be inconvenient):
///
///     #[derive(Ord, PartialOrd)]
///     enum Kind { A, B, C, D }
///     struct Bar<R> { map: BTreeMap<Kind, Ty<R>> }
///     walkable!(Bar<R> => [map]); // requires Kind : Walkable<R>
///     walkable!(Kind);
///
/// This leaf-node use expands to:
///
///     impl<R: Reason> Walkable<R> for Kind {}
macro_rules! walkable {
    ( @ACCEPT($r:ident, $visit:ident) ) => {
        fn accept(& self, v: &mut dyn $crate::visitor::Visitor<$r>) {
            v.$visit(self);
        }
    };
    ( @STRUCT($r:ident, $reason_bound:path, [$($gen:ident)*], $name:ty, $({$accept:item},)? [$($e:tt)*]) ) => {
        impl<$r: $reason_bound $( , $gen: $crate::visitor::Walkable<$r> )* > $crate::visitor::Walkable<$r> for $name {
            $($accept)*

            #[allow(unused_variables)]
            fn recurse(&self, v: &mut dyn $crate::visitor::Visitor<$r>) {
                $(
                    self.$e.accept(v);
                )*
            }
        }
    };
    ( @ENUM($r:ident, $reason_bound:path, [$($gen:ident)*], $name:ty, $({$accept:item},)? [$( $variant:pat, [$($e:tt)*] )*]) ) => {
        impl<$r: $reason_bound $( , $gen: $crate::visitor::Walkable<$r> )* > $crate::visitor::Walkable<$r> for $name {
            $($accept)*

            #[allow(unused_variables)]
            fn recurse(& self, v: &mut dyn $crate::visitor::Visitor<$r>) {
                match self {
                    $(
                        $variant => {
                            $(
                                $e.accept(v);
                            )*
                        }
                    )*
                }
            }
        }
    };
    ( impl < $r:ident : $bound:path $( , $gen:ident )* $(,)? > for $name:ty as $visit:ident => [ $($e:tt),* $(,)? ] ) => {
        walkable! { @STRUCT($r, $bound, [$($gen)*], $name, {walkable!{ @ACCEPT($r, $visit) }}, [$($e)*]) }
    };
    ( impl < $r:ident : $bound:path $( , $gen:ident )* $(,)? > for $name:ty => [ $($e:tt),* $(,)? ] ) => {
        walkable! { @STRUCT($r, $bound, [$($gen)*], $name, [$($e)*]) }
    };
    ( impl < $r:ident : $bound:path $( , $gen:ident )* $(,)? > for $name:ty as $visit:ident => { $( $variant:pat => [ $($e:tt),* $(,)? ] ),* $(,)? } ) => {
        walkable! { @ENUM($r, $crate::reason::Reason, [$($gen)*], $name, {walkable!{ @ACCEPT($r, $visit) }}, [$($variant, [$($e)*])*]) }
    };
    ( impl < $r:ident : $bound:path $( , $gen:ident )* $(,)? > for $name:ty => { $( $variant:pat => [ $($e:tt),* $(,)? ] ),* $(,)? } ) => {
        walkable! { @ENUM($r, $crate::reason::Reason, [$($gen)*], $name, [$($variant, [$($e)*])*]) }
    };
    ( $name:ty as $visit:ident => [ $($e:tt),* $(,)? ] ) => {
        walkable! { @STRUCT(R, $crate::reason::Reason, [], $name, {walkable!{ @ACCEPT(R, $visit) }}, [$($e)*]) }
    };
    ( $name:ty => [ $($e:tt),* $(,)? ] ) => {
        walkable! { @STRUCT(R, $crate::reason::Reason, [], $name, [$($e)*]) }
    };
    ( $name:ty as $visit:ident => { $( $variant:pat => [ $($e:tt),* $(,)? ] ),* $(,)? } ) => {
        walkable! { @ENUM(R, $crate::reason::Reason, [], $name, {walkable!{ @ACCEPT(R, $visit) }}, [$($variant, [$($e)*])*]) }
    };
    ( $name:ty => { $( $variant:pat => [ $($e:tt),* $(,)? ] ),* $(,)? } ) => {
        walkable! { @ENUM(R, $crate::reason::Reason, [], $name, [$($variant, [$($e)*])*]) }
    };
    ( $name:ty as $visit:ident) => {
        walkable! { @STRUCT(R, $crate::reason::Reason, [], $name, {walkable!{ @ACCEPT(R, $visit) }}, []) }
    };
    ( $name:ty ) => {
        walkable! { @STRUCT(R, $crate::reason::Reason, [], $name, []) }
    };
}

walkable!(isize);
walkable!(bool);
walkable!(String);

walkable!(impl<R: Reason, A, B> for (A, B) => [0, 1]);
walkable!(impl<R: Reason, A, B, C> for (A, B, C) => [0, 1, 2]);
walkable!(impl<R: Reason, A, B, C, D> for (A, B, C, D) => [0, 1, 2, 3]);

walkable!(oxidized::file_info::Mode);

walkable!(pos::Symbol as visit_symbol => []);
walkable!(pos::TypeName as visit_type_name => [0]);
walkable!(pos::ModuleName as visit_module_name => [0]);
walkable!(pos::ConstName as visit_const_name => [0]);
walkable!(pos::FunName as visit_fun_name => [0]);
walkable!(pos::ClassConstName as visit_class_const_name => [0]);
walkable!(pos::TypeConstName as visit_type_const_name => [0]);
walkable!(pos::MethodName as visit_method_name => [0]);
walkable!(pos::PropName as visit_prop_name => [0]);

walkable!(crate::decl::ty::ClassishKind);
