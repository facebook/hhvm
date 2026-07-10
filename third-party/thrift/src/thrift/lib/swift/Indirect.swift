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

/// Stores an optional value behind a heap box, breaking the infinite-size cycle
/// for a directly self-referential struct field (Thrift `@thrift.Box`, guide
/// node 2.1.24). Used as a property wrapper: `@Indirect public var next:
/// Node? = nil`.
///
/// Value semantics are preserved: equality and hashing compare the wrapped
/// value (not the box identity), and mutation replaces the box rather than
/// mutating it in place, so struct copies never alias.
@propertyWrapper
public struct Indirect<Wrapped> {
  private final class Box {
    let value: Wrapped
    init(_ value: Wrapped) { self.value = value }
  }

  private var box: Box?

  public var wrappedValue: Wrapped? {
    get { box?.value }
    set { box = newValue.map(Box.init) }
  }

  public init(wrappedValue: Wrapped?) {
    self.box = wrappedValue.map(Box.init)
  }
}

extension Indirect: Equatable where Wrapped: Equatable {
  public static func == (lhs: Indirect, rhs: Indirect) -> Bool {
    lhs.wrappedValue == rhs.wrappedValue
  }
}

extension Indirect: Hashable where Wrapped: Hashable {
  public func hash(into hasher: inout Hasher) {
    hasher.combine(wrappedValue)
  }
}
