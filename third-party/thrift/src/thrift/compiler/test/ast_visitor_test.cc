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

#include <thrift/compiler/ast/ast_visitor.h>

#include <cctype>
#include <memory>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/test/ast_testing.h>

namespace apache::thrift::compiler {
namespace {

class VisitorContextTest : public test::BaseVisitorTest {};

// A helper class for keeping track of visitation expectations.
class MockAstVisitor {
 public:
  MOCK_METHOD(void, visit_interface, (const t_interface*));
  MOCK_METHOD(void, visit_structured_definition, (const t_structured*));
  MOCK_METHOD(void, visit_root_definition, (const t_named*));
  MOCK_METHOD(void, visit_definition, (const t_named*));
  MOCK_METHOD(void, visit_container, (const t_container*));

  MOCK_METHOD(void, visit_program, (const t_program*));

  MOCK_METHOD(void, visit_service, (const t_service*));
  MOCK_METHOD(void, visit_interaction, (const t_interaction*));
  MOCK_METHOD(void, visit_function, (const t_function*));
  MOCK_METHOD(void, visit_throws, (const t_throws*));
  MOCK_METHOD(void, visit_sink, (const t_sink*));
  MOCK_METHOD(void, visit_stream, (const t_stream*));

  MOCK_METHOD(void, visit_structured, (const t_structured*));
  MOCK_METHOD(void, visit_union, (const t_union*));
  MOCK_METHOD(void, visit_exception, (const t_exception*));
  MOCK_METHOD(void, visit_field, (const t_field*));
  MOCK_METHOD(void, visit_enum, (const t_enum*));
  MOCK_METHOD(void, visit_enum_value, (const t_enum_value*));
  MOCK_METHOD(void, visit_const, (const t_const*));
  MOCK_METHOD(void, visit_typedef, (const t_typedef*));

  MOCK_METHOD(void, visit_set, (const t_set*));
  MOCK_METHOD(void, visit_list, (const t_list*));
  MOCK_METHOD(void, visit_map, (const t_map*));

  // Registers with all ast_visitor registration functions.
  template <typename V>
  void addTo(V& visitor) {
    visitor.add_interface_visitor(
        [this](const t_interface& node) { visit_interface(&node); });
    visitor.add_structured_definition_visitor([this](const t_structured& node) {
      visit_structured_definition(&node);
    });
    visitor.add_root_definition_visitor(
        [this](const t_named& node) { visit_root_definition(&node); });
    visitor.add_definition_visitor(
        [this](const t_named& node) { visit_definition(&node); });
    visitor.add_container_visitor(
        [this](const t_container& node) { visit_container(&node); });

    visitor.add_program_visitor(
        [this](const t_program& node) { visit_program(&node); });

    visitor.add_service_visitor(
        [this](const t_service& node) { visit_service(&node); });
    visitor.add_interaction_visitor(
        [this](const t_interaction& node) { visit_interaction(&node); });
    visitor.add_function_visitor(
        [this](const t_function& node) { visit_function(&node); });
    visitor.add_throws_visitor(
        [this](const t_throws& node) { visit_throws(&node); });
    visitor.add_sink_visitor([this](const t_sink& node) { visit_sink(&node); });
    visitor.add_stream_visitor(
        [this](const t_stream& node) { visit_stream(&node); });

    visitor.add_struct_visitor(
        [this](const t_struct& node) { visit_structured(&node); });
    visitor.add_union_visitor(
        [this](const t_union& node) { visit_union(&node); });
    visitor.add_exception_visitor(
        [this](const t_exception& node) { visit_exception(&node); });
    visitor.add_field_visitor(
        [this](const t_field& node) { visit_field(&node); });
    visitor.add_enum_visitor([this](const t_enum& node) { visit_enum(&node); });
    visitor.add_enum_value_visitor(
        [this](const t_enum_value& node) { visit_enum_value(&node); });
    visitor.add_const_visitor(
        [this](const t_const& node) { visit_const(&node); });
    visitor.add_typedef_visitor(
        [this](const t_typedef& node) { visit_typedef(&node); });

    visitor.add_set_visitor([this](const t_set& node) { visit_set(&node); });
    visitor.add_list_visitor([this](const t_list& node) { visit_list(&node); });
    visitor.add_map_visitor([this](const t_map& node) { visit_map(&node); });
  }
};

// A visitor for all node types, that forwards to the type-specific
// MockAstVisitor functions.
class OverloadedVisitor {
 public:
  explicit OverloadedVisitor(MockAstVisitor* mock) : mock_(mock) {}

  void operator()(const t_program&) {}

  void operator()(const t_service& node) { mock_->visit_service(&node); }
  void operator()(const t_interaction& node) {
    mock_->visit_interaction(&node);
  }
  void operator()(const t_function& node) { mock_->visit_function(&node); }
  void operator()(const t_throws& node) { mock_->visit_throws(&node); }

  void operator()(const t_structured& node) { mock_->visit_structured(&node); }
  void operator()(const t_union& node) { mock_->visit_union(&node); }
  void operator()(const t_exception& node) { mock_->visit_exception(&node); }
  void operator()(const t_field& node) { mock_->visit_field(&node); }
  void operator()(const t_enum& node) { mock_->visit_enum(&node); }
  void operator()(const t_enum_value& node) { mock_->visit_enum_value(&node); }
  void operator()(const t_const& node) { mock_->visit_const(&node); }
  void operator()(const t_typedef& node) { mock_->visit_typedef(&node); }

  void operator()(const t_set& node) { mock_->visit_set(&node); }
  void operator()(const t_list& node) { mock_->visit_list(&node); }
  void operator()(const t_map& node) { mock_->visit_map(&node); }
  void operator()(const t_sink& node) { mock_->visit_sink(&node); }
  void operator()(const t_stream& node) { mock_->visit_stream(&node); }

 private:
  MockAstVisitor* mock_;
};

template <typename V>
class AstVisitorTest : public test::BaseProgramTest {
 public:
  AstVisitorTest() noexcept : overload_visitor_(&overload_mock_) {}

  void SetUp() override {
    // Register mock_ to verify add_* function -> nodes visited
    // relationship.
    mock_.addTo(visitor_);
    // Register overload_visitor_ with each multi-node visitor function to
    // verify the correct overloads are used.
    visitor_.add_interface_visitor(overload_visitor_);
    visitor_.add_structured_definition_visitor(overload_visitor_);
    visitor_.add_root_definition_visitor(overload_visitor_);
    visitor_.add_definition_visitor(overload_visitor_);
    visitor_.add_container_visitor(overload_visitor_);

    // Add baseline expectations.
    EXPECT_CALL(this->mock_, visit_program(&this->program_));
    EXPECT_CALL(this->mock_, visit_definition(&this->program_));
  }

  void TearDown() override { visitor_(program_); }

 protected:
  ::testing::StrictMock<MockAstVisitor> mock_;
  ::testing::StrictMock<MockAstVisitor> overload_mock_;
  t_scope scope_;

 private:
  V visitor_;
  OverloadedVisitor overload_visitor_;
};

using AstVisitorTypes = ::testing::Types<ast_visitor, const_ast_visitor>;
TYPED_TEST_SUITE(AstVisitorTest, AstVisitorTypes);

TYPED_TEST(AstVisitorTest, EmptyProgram) {}

TYPED_TEST(AstVisitorTest, Interaction) {
  auto interaction =
      std::make_unique<t_interaction>(&this->program_, "Interaction");
  auto void_ref = t_type_ref::from_req_ptr(&t_base_type::t_void());
  auto func1 =
      std::make_unique<t_function>(&this->program_, void_ref, "function1");
  func1->set_exceptions(std::make_unique<t_throws>());
  auto func2 =
      std::make_unique<t_function>(&this->program_, void_ref, "function2");

  EXPECT_CALL(this->mock_, visit_function(func1.get()));
  EXPECT_CALL(this->mock_, visit_throws(func1->exceptions()));
  EXPECT_CALL(this->mock_, visit_function(func2.get()));
  // Matches: definition.
  EXPECT_CALL(this->mock_, visit_definition(func1.get()));
  EXPECT_CALL(this->mock_, visit_definition(func2.get()));
  EXPECT_CALL(this->overload_mock_, visit_function(func1.get()));
  EXPECT_CALL(this->overload_mock_, visit_function(func2.get()));
  interaction->add_function(std::move(func1));
  interaction->add_function(std::move(func2));

  EXPECT_CALL(this->mock_, visit_interaction(interaction.get()));
  // Matches: interface, root definition, definition.
  EXPECT_CALL(this->mock_, visit_interface(interaction.get()));
  EXPECT_CALL(this->mock_, visit_root_definition(interaction.get()));
  EXPECT_CALL(this->mock_, visit_definition(interaction.get()));
  EXPECT_CALL(this->overload_mock_, visit_interaction(interaction.get()))
      .Times(3);
  this->program_.add_interaction(std::move(interaction));
}

TYPED_TEST(AstVisitorTest, Service) {
  auto service = std::make_unique<t_service>(&this->program_, "Service");
  auto void_ref = t_type_ref::from_req_ptr(&t_base_type::t_void());
  auto func1 =
      std::make_unique<t_function>(&this->program_, void_ref, "function1");
  func1->set_exceptions(std::make_unique<t_throws>());
  auto func2 =
      std::make_unique<t_function>(&this->program_, void_ref, "function2");

  EXPECT_CALL(this->mock_, visit_function(func1.get()));
  EXPECT_CALL(this->mock_, visit_throws(func1->exceptions()));
  EXPECT_CALL(this->mock_, visit_function(func2.get()));
  // Matches: definition.
  EXPECT_CALL(this->mock_, visit_definition(func1.get()));
  EXPECT_CALL(this->mock_, visit_definition(func2.get()));
  EXPECT_CALL(this->overload_mock_, visit_function(func1.get()));
  EXPECT_CALL(this->overload_mock_, visit_function(func2.get()));
  service->add_function(std::move(func1));
  service->add_function(std::move(func2));

  EXPECT_CALL(this->mock_, visit_service(service.get()));
  // Matches: interface, root definition, definition.
  EXPECT_CALL(this->mock_, visit_interface(service.get()));
  EXPECT_CALL(this->mock_, visit_root_definition(service.get()));
  EXPECT_CALL(this->mock_, visit_definition(service.get()));
  EXPECT_CALL(this->overload_mock_, visit_service(service.get())).Times(3);
  this->program_.add_service(std::move(service));
}

TYPED_TEST(AstVisitorTest, Struct) {
  auto tstruct = std::make_unique<t_struct>(&this->program_, "Struct");
  auto field =
      std::make_unique<t_field>(&t_base_type::t_i32(), "struct_field", 1);
  EXPECT_CALL(this->mock_, visit_field(field.get()));
  // Matches: definition.
  EXPECT_CALL(this->mock_, visit_definition(field.get()));
  EXPECT_CALL(this->overload_mock_, visit_field(field.get()));
  tstruct->append(std::move(field));

  EXPECT_CALL(this->mock_, visit_structured(tstruct.get()));
  // Matches: structured_definition, root definition, definition.
  EXPECT_CALL(this->mock_, visit_structured_definition(tstruct.get()));
  EXPECT_CALL(this->mock_, visit_root_definition(tstruct.get()));
  EXPECT_CALL(this->mock_, visit_definition(tstruct.get()));
  EXPECT_CALL(this->overload_mock_, visit_structured(tstruct.get())).Times(3);
  this->program_.add_struct(std::move(tstruct));
}

TYPED_TEST(AstVisitorTest, Union) {
  auto tunion = std::make_unique<t_union>(&this->program_, "Union");
  auto field =
      std::make_unique<t_field>(&t_base_type::t_i32(), "union_field", 1);
  EXPECT_CALL(this->mock_, visit_field(field.get()));
  // Matches: definition.
  EXPECT_CALL(this->mock_, visit_definition(field.get()));
  EXPECT_CALL(this->overload_mock_, visit_field(field.get()));
  tunion->append(std::move(field));

  EXPECT_CALL(this->mock_, visit_union(tunion.get()));
  // Matches: structured_definition, root definition, definition.
  EXPECT_CALL(this->mock_, visit_structured_definition(tunion.get()));
  EXPECT_CALL(this->mock_, visit_root_definition(tunion.get()));
  EXPECT_CALL(this->mock_, visit_definition(tunion.get()));
  EXPECT_CALL(this->overload_mock_, visit_union(tunion.get())).Times(3);
  this->program_.add_struct(std::move(tunion));
}

TYPED_TEST(AstVisitorTest, Exception) {
  auto except = std::make_unique<t_exception>(&this->program_, "Exception");
  auto field =
      std::make_unique<t_field>(&t_base_type::t_i32(), "exception_field", 1);
  EXPECT_CALL(this->mock_, visit_field(field.get()));
  // Matches: definition.
  EXPECT_CALL(this->mock_, visit_definition(field.get()));
  EXPECT_CALL(this->overload_mock_, visit_field(field.get()));
  except->append(std::move(field));

  EXPECT_CALL(this->mock_, visit_exception(except.get()));
  // Matches: structured_definition, root definition, definition.
  EXPECT_CALL(this->mock_, visit_structured_definition(except.get()));
  EXPECT_CALL(this->mock_, visit_root_definition(except.get()));
  EXPECT_CALL(this->mock_, visit_definition(except.get()));
  EXPECT_CALL(this->overload_mock_, visit_exception(except.get())).Times(3);
  this->program_.add_exception(std::move(except));
}

TYPED_TEST(AstVisitorTest, Enum) {
  auto tenum = std::make_unique<t_enum>(&this->program_, "Enum");
  auto tenum_value = std::make_unique<t_enum_value>("EnumValue");
  EXPECT_CALL(this->mock_, visit_enum_value(tenum_value.get()));
  // Matches: definition.
  EXPECT_CALL(this->mock_, visit_definition(tenum_value.get()));
  EXPECT_CALL(this->overload_mock_, visit_enum_value(tenum_value.get()));
  tenum->append(std::move(tenum_value));

  EXPECT_CALL(this->mock_, visit_enum(tenum.get()));
  // Matches: root definition, definition.
  EXPECT_CALL(this->mock_, visit_root_definition(tenum.get()));
  EXPECT_CALL(this->mock_, visit_definition(tenum.get()));
  EXPECT_CALL(this->overload_mock_, visit_enum(tenum.get())).Times(2);
  this->program_.add_enum(std::move(tenum));
}

TYPED_TEST(AstVisitorTest, Const) {
  auto tconst = std::make_unique<t_const>(
      &this->program_, &t_base_type::t_i32(), "Const", nullptr);
  EXPECT_CALL(this->mock_, visit_const(tconst.get()));
  // Matches: root definition, definition.
  EXPECT_CALL(this->mock_, visit_root_definition(tconst.get()));
  EXPECT_CALL(this->mock_, visit_definition(tconst.get()));
  EXPECT_CALL(this->overload_mock_, visit_const(tconst.get())).Times(2);
  this->program_.add_const(std::move(tconst));
}

TYPED_TEST(AstVisitorTest, Typedef) {
  auto ttypedef = std::make_unique<t_typedef>(
      &this->program_, &t_base_type::t_i32(), "Typedef", nullptr);
  EXPECT_CALL(this->mock_, visit_typedef(ttypedef.get()));
  // Matches: root definition, definition.
  EXPECT_CALL(this->mock_, visit_root_definition(ttypedef.get()));
  EXPECT_CALL(this->mock_, visit_definition(ttypedef.get()));
  EXPECT_CALL(this->overload_mock_, visit_typedef(ttypedef.get())).Times(2);
  this->program_.add_typedef(std::move(ttypedef));
}

TYPED_TEST(AstVisitorTest, Set) {
  auto set = std::make_unique<t_set>(&t_base_type::t_i32());
  EXPECT_CALL(this->mock_, visit_set(set.get()));
  EXPECT_CALL(this->mock_, visit_container(set.get()));
  EXPECT_CALL(this->overload_mock_, visit_set(set.get()));
  this->program_.add_type_instantiation(std::move(set));
}

TYPED_TEST(AstVisitorTest, List) {
  auto list = std::make_unique<t_list>(&t_base_type::t_i32());
  EXPECT_CALL(this->mock_, visit_list(list.get()));
  EXPECT_CALL(this->mock_, visit_container(list.get()));
  EXPECT_CALL(this->overload_mock_, visit_list(list.get()));
  this->program_.add_type_instantiation(std::move(list));
}

TYPED_TEST(AstVisitorTest, Map) {
  auto map =
      std::make_unique<t_map>(&t_base_type::t_i32(), &t_base_type::t_i32());
  EXPECT_CALL(this->mock_, visit_map(map.get()));
  EXPECT_CALL(this->mock_, visit_container(map.get()));
  EXPECT_CALL(this->overload_mock_, visit_map(map.get()));
  this->program_.add_type_instantiation(std::move(map));
}

TEST(AstVisitorTest, Sink) {
  auto sink1 =
      std::make_unique<t_sink>(t_base_type::t_i32(), t_base_type::t_i32());
  auto sink1ptr = sink1.get();
  sink1->set_sink_exceptions(std::make_unique<t_throws>());
  auto sink2 =
      std::make_unique<t_sink>(t_base_type::t_i32(), t_base_type::t_i32());
  auto sink2ptr = sink2.get();
  sink2->set_final_response_exceptions(std::make_unique<t_throws>());

  auto program = t_program("path/to/program.thrift");
  auto service = std::make_unique<t_service>(&program, "Service");
  auto ret = t_type_ref();
  service->add_function(std::make_unique<t_function>(
      &program, ret, "f1", nullptr, std::move(sink1)));
  service->add_function(std::make_unique<t_function>(
      &program, ret, "f2", nullptr, std::move(sink2)));
  program.add_service(std::move(service));

  auto visitor = ast_visitor();
  auto responses = std::vector<const t_sink*>();
  visitor.add_sink_visitor(
      [&](const t_sink& node) { responses.push_back(&node); });
  auto throws = std::vector<const t_throws*>();
  visitor.add_throws_visitor(
      [&](const t_throws& node) { throws.push_back(&node); });
  visitor(program);

  EXPECT_THAT(responses, ::testing::ElementsAre(sink1ptr, sink2ptr));
  EXPECT_THAT(
      throws,
      ::testing::ElementsAre(
          sink1ptr->sink_exceptions(), sink2ptr->final_response_exceptions()));
}

TEST(AstVisitorTest, StreamResponse) {
  auto stream1 = std::make_unique<t_stream>(t_base_type::t_i32());
  auto stream1ptr = stream1.get();
  stream1->set_exceptions(std::make_unique<t_throws>());
  auto stream2 = std::make_unique<t_stream>(t_base_type::t_i32());
  auto stream2ptr = stream2.get();

  auto program = t_program("path/to/program.thrift");
  auto service = std::make_unique<t_service>(&program, "Service");
  auto ret = t_type_ref();
  service->add_function(std::make_unique<t_function>(
      &program, ret, "f1", nullptr, std::move(stream1)));
  service->add_function(std::make_unique<t_function>(
      &program, ret, "f2", nullptr, std::move(stream2)));
  program.add_service(std::move(service));

  auto visitor = ast_visitor();
  auto responses = std::vector<const t_stream*>();
  visitor.add_stream_visitor(
      [&](const t_stream& node) { responses.push_back(&node); });
  auto throws = std::vector<const t_throws*>();
  visitor.add_throws_visitor(
      [&](const t_throws& node) { throws.push_back(&node); });
  visitor(program);

  EXPECT_THAT(responses, ::testing::ElementsAre(stream1ptr, stream2ptr));
  EXPECT_THAT(throws, ::testing::ElementsAre(stream1ptr->exceptions()));
}

TEST(AstVisitorTest, Modifications) {
  t_program program("path/to/program.thrift");
  program.create_def<t_union>(&program, "Union1");
  program.create_def<t_union>(&program, "Union2");
  std::vector<std::string> seen;

  ast_visitor visitor;
  visitor.add_union_visitor([&](t_union& node) {
    seen.emplace_back(node.name());
    // Add some more nodes, which will actually show up in the current
    // traversal.
    if (std::isdigit(node.name().back())) { // Don't recurse indefinitely.
      program.create_def<t_union>(&program, node.name() + "a");
      program.create_def<t_union>(&program, node.name() + "b");
    }
  });
  visitor(program);

  EXPECT_THAT(
      seen,
      ::testing::ElementsAre(
          "Union1", "Union2", "Union1a", "Union1b", "Union2a", "Union2b"));
}

class MockObserver {
 public:
  MOCK_METHOD(void, begin_visit, (t_node&));
  MOCK_METHOD(void, end_visit, (t_node&));
};

class ObserverTest : public test::BaseProgramTest {};

TEST_F(ObserverTest, OrderOfCalls) {
  static_assert(ast_detail::is_observer<MockObserver&>::value, "");
  using test_ast_visitor =
      basic_ast_visitor<false, MockObserver&, int, MockObserver&>;

  auto& tunion = create_def<t_union>("Union");
  auto& field = tunion.create_field(&t_base_type::t_i32(), "union_field", 1);
  MockObserver m1, m2;
  {
    ::testing::InSequence ins;
    EXPECT_CALL(m1, begin_visit(::testing::Ref(program_)));
    EXPECT_CALL(m2, begin_visit(::testing::Ref(program_)));
    EXPECT_CALL(m1, begin_visit(::testing::Ref(tunion)));
    EXPECT_CALL(m2, begin_visit(::testing::Ref(tunion)));
    EXPECT_CALL(m1, begin_visit(::testing::Ref(field)));
    EXPECT_CALL(m2, begin_visit(::testing::Ref(field)));

    // End is called in reverse order.
    EXPECT_CALL(m2, end_visit(::testing::Ref(field)));
    EXPECT_CALL(m1, end_visit(::testing::Ref(field)));
    EXPECT_CALL(m2, end_visit(::testing::Ref(tunion)));
    EXPECT_CALL(m1, end_visit(::testing::Ref(tunion)));
    EXPECT_CALL(m2, end_visit(::testing::Ref(program_)));
    EXPECT_CALL(m1, end_visit(::testing::Ref(program_)));
  }
  test_ast_visitor visitor;
  visitor(m1, 1, m2, program_);
}

TEST_F(ObserverTest, VisitContext) {
  static_assert(ast_detail::is_observer<visitor_context&>::value, "");
  using ctx_ast_visitor = basic_ast_visitor<false, visitor_context&>;

  auto& tunion = create_def<t_union>("Union");
  auto& field = tunion.create_field(&t_base_type::t_i32(), "union_field", 1);

  int calls = 0;
  ctx_ast_visitor visitor;
  visitor.add_program_visitor([&](visitor_context& ctx, t_program& node) {
    EXPECT_EQ(&node, &program_);
    EXPECT_EQ(ctx.parent(), nullptr);
    ++calls;
  });
  visitor.add_union_visitor([&](visitor_context& ctx, t_union& node) {
    EXPECT_EQ(&node, &tunion);
    EXPECT_EQ(ctx.parent(), &program_);
    ++calls;
  });
  visitor.add_field_visitor([&](visitor_context& ctx, t_field& node) {
    EXPECT_EQ(&node, &field);
    EXPECT_EQ(ctx.parent(), &tunion);
    ++calls;
  });

  visitor_context ctx;
  EXPECT_EQ(ctx.parent(), nullptr);
  visitor(ctx, program_);
  EXPECT_EQ(calls, 3);
}

} // namespace
} // namespace apache::thrift::compiler
