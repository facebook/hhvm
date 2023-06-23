pub type BoxA = Box<A>;
pub type RcA = Rc<A>;
pub type ArcA = Arc<A>;
pub type RcOcA = Arc<A>;

pub type StdBoxA = std::boxed::Box<A>;
pub type StdRcA = std::rc::Rc<A>;
pub type StdArcA = std::sync::Arc<A>;
pub type OcamlrepRcOcA = std::sync::Arc<A>;

pub type BoxedTuple = Box<(Box<A>, Box<B>)>;
