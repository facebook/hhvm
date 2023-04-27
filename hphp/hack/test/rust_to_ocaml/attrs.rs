/// type X
#[rust_to_ocaml(attr = "deriving show")]
pub type X = A;

/// type Y
#[rust_to_ocaml(
    prefix = "y_",
    attr = r#"deriving visitors {
        variety = "iter";
        ancestors = ["iter_ab"];
    }"#
)]
pub struct Y {
    /// foo
    #[rust_to_ocaml(attr = "opaque")]
    #[rust_to_ocaml(attr = "visitors.opaque")]
    pub foo: A,
    /// bar
    pub bar: B,
}

/// type Visibility
#[rust_to_ocaml(prefix = "V", attr = "deriving eq, ord, show { with_path = false }")]
enum Visibility {
    /// Private
    #[rust_to_ocaml(attr = "visitors.name \"visibility_VPrivate\"")]
    Private,
    /// Public
    #[rust_to_ocaml(attr = r#"visitors.name "visibility_VPublic""#)]
    Public,
}
