pub const FT_FLAGS_RETURN_DISPOSABLE: isize = 0x1;

pub const FT_FLAGS_RETURNS_MUTABLE: isize = 0x2;

pub const FT_FLAGS_RETURNS_VOID_TO_RX: isize = 0x4;

pub const FT_FLAGS_IS_COROUTINE: isize = 0x8;

pub const FT_FLAGS_ASYNC: isize = 0x10;

pub const FT_FLAGS_GENERATOR: isize = 0x20;

pub const FT_FLAGS_INSTANTIATED_TARGS: isize = 0x100;

// These flags are used for return type on FunType and parameter for FunParam
pub const MUTABLE_FLAGS_OWNED: isize = 0x40;

pub const MUTABLE_FLAGS_BORROWED: isize = 0x80;

pub const MUTABLE_FLAGS_MAYBE: isize = 0xC0;

pub const MUTABLE_FLAGS_MASK: isize = 0xC0;

pub const FP_FLAGS_ACCEPT_DISPOSABLE: isize = 0x1;

pub const FP_FLAGS_INOUT: isize = 0x2;
