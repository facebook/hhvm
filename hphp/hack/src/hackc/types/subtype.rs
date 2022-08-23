use std::borrow::Borrow;
use std::borrow::Cow;

use crate::tyx::Tyx;

pub(crate) fn join<'a>(ty1: Cow<'a, Tyx>, ty2: Cow<'a, Tyx>) -> Cow<'a, Tyx> {
    use Tyx::*;
    match (ty1.borrow(), ty2.borrow()) {
        (t1, t2) if equiv(t1, t2) => ty1,
        (Bottom, _) => ty2,
        (_, Bottom) => ty1,
        _ => Cow::Owned(Mixed),
    }
}

// not very sophisiticated
pub(crate) fn equiv(t1: &Tyx, t2: &Tyx) -> bool {
    t1 == t2
}
