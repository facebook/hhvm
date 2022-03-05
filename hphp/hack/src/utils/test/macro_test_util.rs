// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::{TokenStream, TokenTree};
use syn::Error;

type Result<T> = std::result::Result<T, Error>;

fn mismatch(ta: Option<TokenTree>, tb: Option<TokenTree>, ax: &TokenStream, bx: &TokenStream) -> ! {
    panic!(
        "Mismatch!\nLeft:  {}\nRight: {}\nwhen comparing\nLeft:  {}\nRight: {}\n",
        ta.map_or("None".into(), |t| t.to_string()),
        tb.map_or("None".into(), |t| t.to_string()),
        ax,
        bx
    );
}

pub fn assert_pat_eq(a: Result<TokenStream>, b: TokenStream) {
    let a = match a {
        Err(err) => {
            panic!("Unexpected error '{}'", err);
        }
        Ok(ok) => ok,
    };

    fn inner_cmp(a: TokenStream, b: TokenStream, ax: &TokenStream, bx: &TokenStream) {
        let mut ia = a.into_iter();
        let mut ib = b.into_iter();

        loop {
            let t_a = ia.next();
            let t_b = ib.next();
            if t_a.is_none() && t_b.is_none() {
                break;
            }
            if t_a.is_none() || t_b.is_none() {
                mismatch(t_a, t_b, ax, bx);
            }
            let t_a = t_a.unwrap();
            let t_b = t_b.unwrap();

            match (&t_a, &t_b) {
                (TokenTree::Ident(_), TokenTree::Ident(_))
                | (TokenTree::Literal(_), TokenTree::Literal(_))
                | (TokenTree::Punct(_), TokenTree::Punct(_))
                    if t_a.to_string() == t_b.to_string() => {}
                (TokenTree::Group(ga), TokenTree::Group(gb)) => {
                    inner_cmp(ga.stream(), gb.stream(), ax, bx);
                }
                (TokenTree::Group(_), _)
                | (TokenTree::Ident(_), _)
                | (TokenTree::Punct(_), _)
                | (TokenTree::Literal(_), _) => mismatch(Some(t_a), Some(t_b), ax, bx),
            }
        }
    }

    inner_cmp(a.clone(), b.clone(), &a, &b);
}

#[allow(dead_code)]
pub(crate) fn assert_error(a: Result<TokenStream>, b: &str) {
    match a {
        Ok(a) => panic!("Expected error but got:\n{}", a),
        Err(e) => {
            let a = format!("{}", e);
            assert_eq!(a, b, "Incorrect error")
        }
    }
}
