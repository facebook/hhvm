use ocaml::caml;
use std::time::{SystemTime, UNIX_EPOCH};

trait OCamlTransferableStruct {}

struct InterlanguageStruct {
    id: u128,
    value: u32,
}
impl InterlanguageStruct {
    fn new(value: u32) -> Self {
        let id = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_nanos();
        Self { id, value }
    }
}
impl Drop for InterlanguageStruct {
    fn drop(&mut self) {
        println!(
            "{}: drop() received value {}",
            self.id.to_string(),
            self.value.to_string(),
        );
    }
}
impl OCamlTransferableStruct for InterlanguageStruct {}

extern "C" fn generic_finalization_routine<T>(arg: ocaml::core::Value)
where
    T: OCamlTransferableStruct,
{
    println!("Running finalizer (dropping Box)");
    unmarshall_private::<T>(ocaml::Value::new(arg));
}

fn marshall<T>(x: T) -> ocaml::Value
where
    T: OCamlTransferableStruct,
{
    let x = Box::new(Some(x));
    let raw_ptr = Box::leak(x);
    ocaml::Value::alloc_custom(raw_ptr, generic_finalization_routine::<T>)
}

fn unmarshall_private<T>(arg: ocaml::Value) -> Box<Option<T>>
where
    T: OCamlTransferableStruct,
{
    unsafe {
        let raw_ptr = *(arg.custom_ptr_val_mut::<*mut Option<T>>());
        Box::from_raw(raw_ptr)
    }
}

fn unmarshall<T, F>(arg: ocaml::Value, mut f: F)
where
    T: OCamlTransferableStruct,
    F: FnMut(&mut Option<T>),
{
    let mut x = unmarshall_private(arg);
    f(x.as_mut());
    Box::leak(x);
}

fn create_the_struct() -> InterlanguageStruct {
    let x = InterlanguageStruct::new(3);
    println!(
        "{}: create_the_struct() set value to {}",
        x.id.to_string(),
        x.value.to_string(),
    );
    x
}

fn use_the_struct(x: &mut Option<InterlanguageStruct>) {
    if let Some(ref mut x) = x {
        let new_value = 5;
        println!(
            "{}: use_the_struct() received value {}; changing it to {}",
            x.id.to_string(),
            x.value.to_string(),
            new_value.to_string(),
        );
        x.value = new_value;
    } else {
        println!("unknown: use_the_struct() received a struct that was already dropped");
    }
}

fn drop_the_struct(x: &mut Option<InterlanguageStruct>) {
    if let Some(x) = x {
        println!("{}: drop_the_struct() called", x.id.to_string());
    } else {
        println!("unknown: drop_the_struct() called on a struct that was already dropped");
    }
    // to take ownership of the struct while in use (and thus be able to drop it):
    std::mem::replace(x, None);
}

caml!(create_the_struct_ocaml() {
    marshall(create_the_struct())
});

caml!(use_the_struct_ocaml(arg) {
    unmarshall(arg, use_the_struct);
    ocaml::Value::unit()
});

caml!(drop_the_struct_ocaml(arg) {
    unmarshall(arg, drop_the_struct);
    ocaml::Value::unit()
});
