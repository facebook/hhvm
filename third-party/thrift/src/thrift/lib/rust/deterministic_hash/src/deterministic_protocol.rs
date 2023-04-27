/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use anyhow::Result;
use fbthrift::protocol::ProtocolWriter;
use fbthrift::thrift_protocol::MessageType;
use fbthrift::ttype::TType;

use crate::DeterministicAccumulator;
use crate::Hasher;

pub struct DeterministicProtocolSerializer<T: Hasher, F: Fn() -> T> {
    acc: DeterministicAccumulator<T, F>,
    maps_is_empty_stack: Vec<bool>,
}

impl<T: Hasher, F: Fn() -> T> DeterministicProtocolSerializer<T, F> {
    pub fn new(generator: F) -> DeterministicProtocolSerializer<T, F> {
        DeterministicProtocolSerializer {
            acc: DeterministicAccumulator::<T, F>::new(generator),
            maps_is_empty_stack: Vec::new(),
        }
    }
    fn begin_container(&mut self, size: i32) {
        self.acc.begin_ordered();
        self.acc.combine(size);
    }
    fn end_container(&mut self) {
        self.acc.end_ordered()
    }
    fn combine_buf(&mut self, buffer: &[u8]) {
        self.acc.begin_ordered();
        self.acc.combine(buffer.len() as i32);
        self.acc.combine(buffer);
        self.acc.end_ordered();
    }
}

impl<T: Hasher, F: Fn() -> T> ProtocolWriter for DeterministicProtocolSerializer<T, F> {
    type Final = Result<T::Output>;

    fn write_message_begin(&mut self, _name: &str, _type_id: MessageType, _seqid: u32) {}
    fn write_message_end(&mut self) {}
    fn write_struct_begin(&mut self, _name: &str) {
        self.acc.begin_unordered();
    }
    fn write_struct_end(&mut self) {
        self.acc.end_unordered();
    }
    fn write_field_begin(&mut self, _name: &str, _type_id: TType, id: i16) {
        self.acc.begin_ordered();
        self.write_i16(id);
    }
    fn write_field_end(&mut self) {
        self.acc.end_ordered();
    }
    fn write_field_stop(&mut self) {}
    fn write_map_begin(&mut self, _key_type: TType, _value_type: TType, size: usize) {
        self.begin_container(size.try_into().unwrap());
        self.acc.begin_unordered();
        self.maps_is_empty_stack.push(true);
    }
    fn write_map_key_begin(&mut self) {
        let map_is_empty = self
            .maps_is_empty_stack
            .last_mut()
            .expect("map stack is empty during map key begin");
        if *map_is_empty {
            *map_is_empty = false; // it's not empty any more
        } else {
            self.acc.end_ordered(); // we had a previous kv pair
        }
        self.acc.begin_ordered();
    }
    fn write_map_value_begin(&mut self) {}
    fn write_map_end(&mut self) {
        let map_is_empty = self
            .maps_is_empty_stack
            .pop()
            .expect("map stack is empty on map end");
        if !map_is_empty {
            self.acc.end_ordered();
        }
        self.acc.end_unordered();
        self.end_container();
    }
    fn write_list_begin(&mut self, _elem_type: TType, size: usize) {
        self.begin_container(size.try_into().unwrap());
        self.acc.begin_ordered();
    }
    fn write_list_value_begin(&mut self) {}
    fn write_list_end(&mut self) {
        self.acc.end_ordered();
        self.end_container();
    }
    fn write_set_begin(&mut self, _elem_type: TType, size: usize) {
        self.begin_container(size as i32);
        self.acc.begin_unordered();
    }
    fn write_set_value_begin(&mut self) {}
    fn write_set_end(&mut self) {
        self.acc.end_unordered();
        self.end_container();
    }
    fn write_bool(&mut self, value: bool) {
        self.acc.combine(value);
    }
    fn write_byte(&mut self, value: i8) {
        self.acc.combine(value);
    }
    fn write_i16(&mut self, value: i16) {
        self.acc.combine(value);
    }
    fn write_i32(&mut self, value: i32) {
        self.acc.combine(value);
    }
    fn write_i64(&mut self, value: i64) {
        self.acc.combine(value);
    }
    fn write_double(&mut self, value: f64) {
        self.acc.combine(value);
    }
    fn write_float(&mut self, value: f32) {
        self.acc.combine(value);
    }
    fn write_string(&mut self, value: &str) {
        self.combine_buf(value.as_bytes());
    }
    fn write_binary(&mut self, value: &[u8]) {
        self.combine_buf(value);
    }

    fn finish(self) -> Self::Final {
        self.acc.get_result()
    }
}
