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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/Math.h>

#include <thrift/lib/cpp2/frozen/FrozenUtil.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Reference_layouts.h>

using namespace apache::thrift::frozen;
using namespace apache::thrift::test;

std::unique_ptr<Person> makePerson(const std::string& name) {
  auto p = std::make_unique<Person>();
  *p->name() = name;
  return p;
}

std::unique_ptr<Node> makeNode(int64_t id, const std::string& content) {
  auto n = std::make_unique<Node>();
  *n->id() = id;
  *n->content() = content;
  return n;
}

BoxedNode makeBoxedNode(int64_t id, const std::string& content) {
  BoxedNode b;
  b.id() = id;
  b.content() = content;
  return b;
}

void fillTree(std::unique_ptr<Node>& node, int min, int max) {
  if (min >= max)
    return;
  int mid = folly::midpoint(min, max);
  node = std::make_unique<Node>();
  *node->id() = mid;
  fillTree(node->left(), min, mid);
  fillTree(node->right(), mid + 1, max);
}

const std::string s1 = "sailing";
const std::string s2 = "shenandoah";

TEST(Frozen, simple_ref) {
  SimpleRef t;
  t.c1r() = makePerson("c1r");
  t.c2r() = makePerson("c2r");
  t.c2s_opt() = makePerson("c2s_opt");
  t.c2u_opt() = makePerson("c2u_opt");
  t.c2r_opt() = makePerson("c2r_opt");
  // unset default ref fields have default value
  EXPECT_EQ(*t.c2s()->name(), "");
  EXPECT_EQ(*t.c2u()->name(), "");
  // unset optional ref field is nullptr
  EXPECT_EQ(t.c1r_opt(), nullptr);

  auto f = freeze(t);
  // empty pointers
  EXPECT_EQ(f.c2s()->name(), *t.c2s()->name());
  EXPECT_EQ(f.c2s()->name(), "");
  EXPECT_EQ(f.c2u()->name(), *t.c2u()->name());
  EXPECT_EQ(f.c2u()->name(), "");

  EXPECT_EQ(f.c1r()->name(), "c1r");
  EXPECT_EQ(f.c2r()->name(), "c2r");
  EXPECT_EQ(f.c2s_opt()->name(), "c2s_opt");
  EXPECT_EQ(f.c2u_opt()->name(), "c2u_opt");
  EXPECT_EQ(f.c1r_opt(), nullptr);
  EXPECT_EQ(f.c2r_opt()->name(), "c2r_opt");
  EXPECT_EQ(f.thaw(), t);

  auto str = freezeToString(t);
  auto f2 = mapFrozen<SimpleRef>(std::move(str));
  EXPECT_EQ(f2.c2s()->name(), *t.c2s()->name());
  EXPECT_EQ(f2.c2s()->name(), "");
  EXPECT_EQ(f2.c2u()->name(), *t.c2u()->name());
  EXPECT_EQ(f2.c2u()->name(), "");
  EXPECT_EQ(f2.c1r()->name(), "c1r");
  EXPECT_EQ(f2.c2r()->name(), "c2r");
  EXPECT_EQ(f2.c2s_opt()->name(), "c2s_opt");
  EXPECT_EQ(f2.c2u_opt()->name(), "c2u_opt");
  EXPECT_EQ(f2.c1r_opt(), nullptr);
  EXPECT_EQ(f2.c2r_opt()->name(), "c2r_opt");
  EXPECT_EQ(f2.thaw(), t);
}

TEST(Frozen, boxed) {
  SimpleBoxed t;
  Person p = Person();
  p.name() = "boxed";
  p.id() = 42;
  t.boxed() = std::move(p);
  {
    auto f = freeze(t);
    EXPECT_EQ(f.boxed()->name(), "boxed");
  }
  {
    auto str = freezeToString(t);
    auto f = mapFrozen<SimpleBoxed>(std::move(str));
    EXPECT_EQ(f.boxed()->name(), "boxed");
  }
}

TEST(Frozen, boxed_empty) {
  SimpleBoxed t;
  {
    auto f = freeze(t);
    EXPECT_FALSE(f.boxed());
  }
  {
    auto str = freezeToString(t);
    auto f = mapFrozen<SimpleBoxed>(std::move(str));
    EXPECT_FALSE(f.boxed());
  }
}

TEST(Frozen, recursive_node_ref) {
  Node root;
  *root.id() = 8;
  root.left() = makeNode(3, s1);
  root.right() = makeNode(5, s2);
  *root.content() = s2;
  EXPECT_EQ(root.left()->left(), nullptr);

  auto f = freeze(root);
  EXPECT_EQ(f.id(), 8);
  EXPECT_EQ(f.content(), s2);

  EXPECT_EQ(f.right()->id(), 5);
  EXPECT_EQ(f.right()->content(), s2);
  EXPECT_EQ(f.right()->left(), nullptr);
  EXPECT_EQ(f.right()->right(), nullptr);

  EXPECT_EQ(f.left()->id(), 3);
  EXPECT_EQ(f.left()->content(), s1);
  EXPECT_EQ(f.left()->left(), nullptr);

  // compare nullptr with unset ref
  EXPECT_EQ(nullptr, f.left()->right());

  // compare unset ref with unset ref
  EXPECT_EQ(f.left()->left(), f.left()->right());

  // compare set ref with set ref
  EXPECT_NE(f.left(), f.right());

  // compare set ref itself
  auto&& p = f.left();
  EXPECT_EQ(f.left(), p);
  EXPECT_EQ(p, f.left());

  // bool
  EXPECT_EQ(bool(f.left()), true);
  EXPECT_EQ(bool(f.left()->left()), false);

  EXPECT_EQ(f.thaw(), root);

  auto str = freezeToString(root);
  auto f2 = mapFrozen<Node>(std::move(str));
  EXPECT_EQ(f2.thaw(), root);
}

TEST(Frozen, recursive_boxed_node_ref) {
  BoxedNode root;
  root.id() = 8;
  root.left() = makeBoxedNode(3, s1);
  root.right() = makeBoxedNode(5, s2);
  root.content() = s2;
  EXPECT_FALSE(root.left()->left());

  auto f = freeze(root);
  EXPECT_EQ(f.id(), 8);
  EXPECT_EQ(f.content(), s2);

  EXPECT_EQ(f.right()->id(), 5);
  EXPECT_EQ(f.right()->content(), s2);
  EXPECT_EQ(f.right()->left(), nullptr);
  EXPECT_EQ(f.right()->right(), nullptr);

  EXPECT_EQ(f.left()->id(), 3);
  EXPECT_EQ(f.left()->content(), s1);
  EXPECT_EQ(f.left()->left(), nullptr);

  // compare nullptr with unset ref
  EXPECT_EQ(nullptr, f.left()->right());

  // compare unset ref with unset ref
  EXPECT_EQ(f.left()->left(), f.left()->right());

  // compare set ref with set ref
  EXPECT_NE(f.left(), f.right());

  // compare set ref itself
  auto&& p = f.left();
  EXPECT_EQ(f.left(), p);
  EXPECT_EQ(p, f.left());

  // bool
  EXPECT_EQ(bool(f.left()), true);
  EXPECT_EQ(bool(f.left()->left()), false);

  EXPECT_EQ(f.thaw(), root);

  auto str = freezeToString(root);
  auto f2 = mapFrozen<BoxedNode>(std::move(str));
  EXPECT_EQ(f2.thaw(), root);
}

TEST(Frozen, instance_cycle_1) {
  auto a = std::make_shared<LinkedListNode>();
  *a->id() = 1;
  a->next() = a;

  // Note: It's possible to freeze the shared_ptr directly, not *a.
  auto fa = freeze(a);
  EXPECT_NE(fa->next(), nullptr);
  EXPECT_EQ(fa->next()->next()->next(), fa->next());
  EXPECT_EQ(fa->next()->next(), fa);
  EXPECT_EQ(fa->next()->next()->next()->next(), fa);

  // unloop the thrift object to avoid memory leak. Note no need to unloop the
  // frozen object as we handle the pointers carefully to avoid loop(s)
  a->next() = nullptr;
}

TEST(Frozen, instance_cycle_2) {
  auto a = std::make_shared<LinkedListNode>();
  *a->id() = 1;
  auto b = std::make_shared<LinkedListNode>();
  *b->id() = 2;

  a->next() = b;
  b->next() = a;

  // Note: Freeze the shared_ptr, not *a.
  auto fa = freeze(a);
  EXPECT_NE(fa->next(), nullptr);
  auto fb = freeze(b);
  EXPECT_EQ(fa->next()->next()->next(), fa->next());
  EXPECT_EQ(fa->next()->next(), fa);
  EXPECT_EQ(fb->next()->next()->next(), fb->next());
  EXPECT_EQ(fb->next()->next(), fb);
  EXPECT_NE(fa, fb);
  EXPECT_EQ(fa->next()->next()->next()->next(), fa);

  // unloop the thrift object to avoid memory leak. Note no need to unloop the
  // frozen object as we handle the pointers carefully to avoid loop(s)
  a->next() = nullptr;
}

// 1000 node only takes 4001 byte!! While the nomal obj will need 32 KByte
TEST(Frozen, tree_size) {
  std::unique_ptr<Node> root;
  fillTree(root, 0, 1000);
  CHECK(root);
  auto f = freeze(root);
  //   250
  //     375
  //       438
  // 500
  //   750
  EXPECT_EQ(f->id(), 500);
  EXPECT_EQ(f->left()->id(), 250);
  EXPECT_EQ(f->left()->right()->id(), 375);
  EXPECT_EQ(f->left()->right()->right()->id(), 438);

  EXPECT_EQ(frozenSize(root), 4001);

  // serde
  auto str = freezeToString(root);
  // serialized size is 4095, still very small
  EXPECT_EQ(str.size(), 4095);
  auto f2 = mapFrozen<std::unique_ptr<Node>>(std::move(str));
  EXPECT_EQ(f2->id(), 500);
  EXPECT_EQ(f2->left()->id(), 250);
  EXPECT_EQ(f2->left()->right()->id(), 375);
  EXPECT_EQ(f2->left()->right()->right()->id(), 438);
}

TEST(Frozen, list_size) {
  LinkedListNode first;
  LinkedListNode* current = &first;
  for (size_t i = 1; i <= 100; ++i) {
    current->next() = std::make_unique<LinkedListNode>();
    current = current->next().get();
    *current->id() = i;
  }
  EXPECT_EQ(frozenSize(first), 201);
}

TEST(Frozen, shared_ref) {
  SharedRef t;
  *t.id() = 9527;
  std::shared_ptr<Person> that = makePerson(s2);
  t.p1() = that;
  t.p2() = that;
  EXPECT_EQ(t.p1(), t.p2());

  auto f = freeze(t);
  EXPECT_EQ(f.id(), 9527);
  EXPECT_EQ(f.p1()->name(), s2);
  EXPECT_EQ(f.p2()->name(), s2);
  EXPECT_EQ(f.p1(), f.p2());

  // TODO: The perfect thaw should not only thaw out correct value, but also let
  // p1 and p2 point to the same object, which is not implemented yet.
  EXPECT_EQ(f.thaw(), t);
}

TEST(Frozen, shared_ref_schema_evolution) {
  SharedRef t;
  *t.id() = 9527;
  std::shared_ptr<Person> that = makePerson(s2);
  t.p1() = that;
  t.p2() = that;

  auto str = freezeToString(t);
  auto f2 = mapFrozen<SharedRef2>(std::move(str));
  EXPECT_EQ(f2.id(), 9527);
  EXPECT_EQ(f2.p2()->name(), s2);
}
