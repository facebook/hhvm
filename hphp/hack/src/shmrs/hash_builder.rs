/// The default `HashBuilder` for a `shmrs`-backed table.
///
/// It is important that a HashDOS-resistant hash function initializes
/// the hash state **eagerly, i.e. immediately upon table cration.**
///
/// If not, the hash function might be called lazily (e.g. when the first
/// key-value pair is first inserted/queried). This lazily generation
/// **would happen after forking**, meaning that the **hashing key might
/// **not be shared between processes!**. Each process would be using
/// a different hash key, which would lead to mayhem.
///
/// From version 0.13.2, `hashbrown` uses a `HashBuilder` that initializes
/// the hash state lazily by default, so we have to use one that doesn't.
///
/// `ahash::RandomState` is a good choice, as it initializes its key on
/// the call to `RandomState::new()`, which inevitably happens when the
/// table is created.
///
/// Furthermore, `ahash::RandomState` IS the base implementation for
/// `hashbrown::hash_map::DefaultHashBuilder`, so we can use it directly.
pub type ShmrsHashBuilder = ahash::RandomState;
