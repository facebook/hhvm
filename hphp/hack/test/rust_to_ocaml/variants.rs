pub enum A {
    I,
    J(isize),
    K(isize, isize),
    L((isize, isize)),
    M { x: isize, y: isize },
}

#[rust_to_ocaml(prefix = "P")]
pub enum Prefixed {
    I,
    J(isize),
    K(isize, isize),
    L((isize, isize)),
    #[rust_to_ocaml(prefix = "m_")]
    M {
        x: isize,
        y: isize,
    },
}
