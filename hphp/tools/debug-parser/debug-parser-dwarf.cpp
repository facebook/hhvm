/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#if defined(__linux__) || defined(__FreeBSD__)

#include <folly/Demangle.h>
#include <folly/Format.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/portability/Unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dwarf.h>

#include "hphp/util/assertions.h"
#include "hphp/util/functional.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/tools/debug-parser/debug-parser.h"
#include "hphp/tools/debug-parser/dwarfstate.h"

/*
 * Debug parser for DWARF (using dwarfstate)
 *
 * DWARF is structured as a forest of DIEs (Debug Information Entry). Each DIE
 * has a tag, which describes what kind of DIE it is, and a list of
 * attributes. Each attribute has a type, which identifies what it is, and a
 * value (the type of the value is implied by the attribute type). Furthermore,
 * a DIE can have other DIEs as children. The top-level DIEs correspond to
 * compilation-units, and all the children of these top-level DIEs correspond to
 * the information in that compilation-unit.
 *
 * The meaning and interpretation of the DIEs is deliberately left vague by the
 * standard, so different compilers can encode things in different ways (and no
 * implementation is bug free).
 */

namespace debug_parser { namespace {

TRACE_SET_MOD(trans);

////////////////////////////////////////////////////////////////////////////////

// Allow foreach on a range (as returned by equal_range)
template<typename It> It begin(std::pair<It,It> p) { return p.first; }
template<typename It> It end(std::pair<It,It> p) { return p.second; }

/*
 * Fully qualified names aren't represented explicitly in DWARF. Instead the
 * structure of the DIEs mimics the nesting structure in the source (IE, a
 * nested class within a class nested within a namespace). So, in order to
 * infer the fully qualified name for any given class, the current scope is
 * tracked as the DIEs are walked.
 *
 * Likewise, DWARF has no concept of linkage, but the linkage is needed to know
 * which types are actually equivalent. Luckily, a type's linkage is closely
 * related to its scope (except for templates, see below), so it can be inferred
 * the same way.
 *
 * The scope is tracked as a stack of contexts, pushing and popping off contexts
 * when a namespace or type is entered or exited.
 */

struct Scope {
  explicit Scope(GlobalOff cu_offset)
      : m_cu_offset{cu_offset}
  {
    m_scope.emplace_back(
      ObjectTypeName{std::string{}, ObjectTypeName::Linkage::external},
      true
    );
  }

  GlobalOff cuOffset() const { return m_cu_offset; }

  ObjectTypeName name() const;

  // Fix the name of a type to match where it is in the namespace/type
  // hierarchy.
  void fixName(ObjectTypeName newName);

  ObjectTypeName::Linkage linkage() const {
    return m_scope.back().name.linkage;
  }

  std::size_t unnamedTypeCount() const {
    return m_scope.back().unnamed_count;
  }

  bool isInNamespaceScope() const {
    return m_scope.back().in_namespace_scope;
  }

  void incUnnamedTypeCount() { ++m_scope.back().unnamed_count; }

  folly::Optional<GlobalOff> typeOffset() const {
    return m_scope.back().offset;
  }

  void pushType(std::string name, GlobalOff offset) {
    m_scope.emplace_back(
      ObjectTypeName{std::move(name), linkage()},
      false
    );
    m_scope.back().offset = offset;
  }

  void pushUnnamedType(std::string name, GlobalOff offset) {
    m_scope.emplace_back(
      ObjectTypeName{
        std::move(name),
        ObjectTypeName::Linkage::none
      },
      false
    );
    m_scope.back().offset = offset;
  }

  void pushNamespace(std::string ns) {
    m_scope.emplace_back(
      ObjectTypeName{std::move(ns), linkage()},
      true
    );
  }

  void pushUnnamedNamespace() {
    m_scope.emplace_back(
      ObjectTypeName{
        "(unnamed namespace)",
        ObjectTypeName::Linkage::internal
      },
      true
    );
  }

  void pop() { m_scope.pop_back(); }

private:
  struct Context {
    Context(ObjectTypeName name, bool in_namespace_scope)
        : name(std::move(name))
        , in_namespace_scope{in_namespace_scope} {}
    ObjectTypeName name;
    bool in_namespace_scope;
    std::size_t unnamed_count = 0;
    folly::Optional<GlobalOff> offset;
  };
  std::vector<Context> m_scope;
  GlobalOff m_cu_offset;

public:
  static const std::string s_pseudo_type_name;
};

/*
 * Actual implementation of TypeParser for DWARF.
 */

struct TypeParserImpl : TypeParser {
  explicit TypeParserImpl(const std::string& filename, int num_threads);

  Object getObject(ObjectTypeKey key) override;

  size_t getObjectBlockCount() const override;

protected:
  const std::vector<ObjectType>& getObjectBlock(size_t index) const override;

private:
  struct StateBlock;

  struct LinkageDependents {
    folly::F14FastSet<GlobalOff> template_uses;
    folly::F14FastSet<GlobalOff> children;
  };

  struct StaticSpec {
    static auto constexpr kNoAddress = std::numeric_limits<uint64_t>::max();
    std::string linkage_name;
    uint64_t address{kNoAddress};
    bool is_member{false};
  };

  struct Env {
    const DwarfState* dwarf;
    std::unique_ptr<StateBlock> state;
    folly::F14FastMap<GlobalOff, GlobalOff> local_mappings;
    folly::F14FastMap<GlobalOff, LinkageDependents> linkage_dependents;
    std::vector<std::pair<GlobalOff, StaticSpec>> raw_static_definitions;
  };

  // Functions used while concurrently building state. Since these functions are
  // invoked from multiple threads, they are static and take all their state
  // explicitly as parameters.
  static void genNames(Env& env,
                       Dwarf_Die die,
                       Scope& scope,
                       std::vector<GlobalOff>* template_params = nullptr);

  static folly::Optional<uintptr_t> interpretLocAddress(const DwarfState& dwarf,
                                                        Dwarf_Attribute attr);
  static folly::Optional<GlobalOff> parseSpecification(const DwarfState& dwarf,
                                                       Dwarf_Die die,
                                                       bool first,
                                                       StaticSpec& spec);
  void fixTemplateLinkage();

  // Functions used after state is built. These are not thread-safe.
  Object genObject(Dwarf_Die die,
                   ObjectTypeName name,
                   ObjectTypeKey key);
  Type genType(Dwarf_Die die);
  Object::Member genMember(Dwarf_Die die,
                           const ObjectTypeName& parent_name);
  Object::Function genFunction(Dwarf_Die die);
  Object::Base genBase(Dwarf_Die die, const ObjectTypeName& parent_name);
  Object::TemplateParam genTemplateParam(Dwarf_Die die);
  folly::Optional<size_t> determineArrayBound(Dwarf_Die die);

  void fillFuncArgs(Dwarf_Die die, FuncType& func);

  // Map a given offset to the state block which contains state for that offset
  // (see below).
  const StateBlock& stateForOffset(GlobalOff offset) const {
    assertx(!m_state_map.empty());
    auto it = std::upper_bound(
      m_state_map.begin(),
      m_state_map.end(),
      offset,
      [](GlobalOff offset, const std::pair<GlobalOff, StateBlock*>& p) {
        return offset < p.first;
      }
    );
    if (it != m_state_map.begin()) --it;
    return *it->second;
  }

  // All of the parser's persistent state is stored in some number of
  // blocks. All of the blocks are computed concurrently, one block per
  // thread. To avoid the overhead of merging the blocks together, they are kept
  // separated. Instead m_state_map is used to map a given offset into the block
  // which contains the state for that offset. It is a list of offset/state
  // pairs. Any offset between the offset given in the pair and the one in the
  // next pair is mapped to the state block in the pair.
  //
  // Note: this scheme only works because each compilation unit is
  // self-contained and does not reference data in another compilation
  // unit. However, nothing in DWARF prevents this and its not guaranteed to
  // always be true.
  struct StateBlock {
    std::vector<ObjectType> all_objs;
    folly::F14FastMap<GlobalOff, size_t> obj_offsets;
    std::multimap<GlobalOff, StaticSpec> static_definitions;
  };
  std::vector<std::unique_ptr<StateBlock>> m_states;
  std::vector<std::pair<GlobalOff, StateBlock*>> m_state_map;
  tbb::concurrent_hash_map<GlobalOff,
                           LinkageDependents,
                           GlobalOff::Hash> m_linkage_dependents;

  DwarfState m_dwarf;
};

// Purposefully fake name to avoid confusion with an actual type.
const std::string Scope::s_pseudo_type_name = "@_PSEUDO_TY";

ObjectTypeName Scope::name() const {
  auto iter = m_scope.begin();
  std::string str = iter->name.name;
  ++iter;
  for (; iter != m_scope.end(); ++iter) {
    if (str.empty()) str = iter->name.name;
    else str = folly::sformat("{}::{}", str, iter->name.name);
  }
  return ObjectTypeName{std::move(str), linkage()};
}

void Scope::fixName(ObjectTypeName newName) {
  if (m_scope.size() == 1) {
    m_scope.back().name = std::move(newName);
    return;
  }

  auto context = std::move(m_scope.back());
  m_scope.pop_back();
  auto outerName = name();
  assertx(newName.name.size() > outerName.name.size());
  if (outerName.name.size()) {
    assertx(!outerName.name.compare(0, outerName.name.size(), newName.name));
    newName.name = newName.name.substr(outerName.name.size() + 2);
  }
  context.name = std::move(newName);
  m_scope.push_back(std::move(context));
}

TypeParserImpl::TypeParserImpl(const std::string& filename, int num_threads)
    : m_dwarf{filename}
{
  // Processing each compiliation unit is very expensive, as it involves walking
  // a large part of the debug information. To speed things up (a lot), we buid
  // up the state concurrently. Create a job corresponding to each compiliation
  // unit in the file and enqueue the jobs with a thread pool. We'll find the
  // offsets of the compiliation unit in the main thread, enqueuing them as we
  // find them. This lets us not only exploit concurrency between processing
  // compiliation units, but between finding them and processing them.
  //
  // Each worker maintains its own private state which it populates for all the
  // compiliation units its assigned (each worker can process multiple
  // compiliation units). Once done, all the different states are kept separate
  // (merging them would be too expensive), but a mapping is constructed to map
  // offsets to the appropriate state block.
  //
  // This whole scheme is only viable because (right now), debug information in
  // a given compilation unit doesn't reference anything outside of that unit,
  // so the state for any given compiliation unit can be processed
  // independently.

  // The context serves as the link between a worker and the TypeParserImpl
  // state (this is forced by the JobQueueWorker interface).
  struct Context {
    const decltype(m_dwarf)& dwarf;
    decltype(m_states)& states;
    decltype(m_state_map)& state_map;
    decltype(m_linkage_dependents)& linkage_dependents;
    // The lock protects states, state_map, and the exception field (but only
    // when the workers are running).
    std::mutex lock;
    // Set to the exception if any of the workers threw (first one wins).
    std::exception_ptr exception;
  };

  // Thread worker. We'll end up with a state block for each one of these.
  struct Worker : HPHP::JobQueueWorker<GlobalOff, Context*> {
    Env env;

    // Remember each offset we processed so we can record it the global state
    // map when we finish.
    std::vector<GlobalOff> offsets;

    void doJob(GlobalOff offset) override {
      // Process a compiliation unit at the given offset.
      try {
        // We're going to use it so let's mark this worker active.
        if (!env.dwarf) {
          env.dwarf = &m_context->dwarf;
          env.state = std::make_unique<StateBlock>();
        }

        offsets.emplace_back(offset);

        // Do the actual processing, adding to the state block:
        Scope scope{offset};
        env.dwarf->onDIEAtOffset(
          offset,
          [&](Dwarf_Die cu) { genNames(env, cu, scope); }
        );

        auto const remap = [&] (GlobalOff o) {
          auto const it = env.local_mappings.find(o);
          if (it != env.local_mappings.end()) {
            return it->second;
          }
          return o;
        };

        // Generate static_definitions by updating their keys collected during
        // genNames. Some keys refer back to a DW_AT_member that belongs to a
        // struct whose definition was in another type-unit. We want to add an
        // entry for the member in the definition.
        std::transform(
            env.raw_static_definitions.begin(),
            env.raw_static_definitions.end(),
            std::inserter(
                env.state->static_definitions,
                env.state->static_definitions.end()),
            [&](const auto& elem) {
              return std::make_pair(remap(elem.first), std::move(elem.second));
            });
        env.raw_static_definitions.clear();

        for (auto& linkage : env.linkage_dependents) {
          if (!linkage.second.template_uses.size()) continue;

          std::decay_t<decltype(m_context->linkage_dependents)>::accessor acc;

          auto const inserted =
            m_context->linkage_dependents.insert(acc, remap(linkage.first));
          if (inserted && !env.local_mappings.size()) {
            acc->second = std::move(linkage.second);
          } else {
            auto const process = [&] (auto const& from, auto& to) {
              for (auto& elm : from) {
                to.insert(remap(elm));
              }
            };
            process(linkage.second.template_uses, acc->second.template_uses);
            process(linkage.second.children, acc->second.children);
          }
        }
        env.linkage_dependents.clear();
        env.local_mappings.clear();
      } catch (...) {
        // Store any exception thrown so it can be rethrown in the main
        // thread. We only bother to store the first one.
        stop();
        std::lock_guard<std::mutex> guard{m_context->lock};
        if (!m_context->exception) {
          m_context->exception = std::current_exception();
        }
      }
    }

    void onThreadExit() override {
      // The worker is done (we've been told to stop). Now that we know we won't
      // be processing anymore offsets, do the needed post-processing on the
      // rest of the state.
      if (!env.dwarf) return;
      try {
        // Compute a mapping of an object type's offset to its location in the
        // all_objs vector.
        env.state->obj_offsets.reserve(env.state->all_objs.size());
        for (auto i = size_t{0}; i < env.state->all_objs.size(); ++i) {
          env.state->obj_offsets.emplace(
            GlobalOff::fromRaw(env.state->all_objs[i].key.object_id), i
          );
        }

        // Record all the offsets this worker processed (along with the state
        // block) in the global state map. This is done using a lock because its
        // quick and only done when the thread is finishing.
        std::lock_guard<std::mutex> guard{m_context->lock};
        auto const state = env.state.get();
        m_context->states.emplace_back(std::move(env.state));
        for (auto offset : offsets) {
          m_context->state_map.emplace_back(offset, state);
        }
      } catch (...) {
        // Store any exception thrown so it can be rethrown in the main
        // thread. We only bother to store the first one.
        stop();
        std::lock_guard<std::mutex> guard{m_context->lock};
        if (!m_context->exception) {
          m_context->exception = std::current_exception();
        }
      }
    }
  };

  // Create the thread pool
  Context context{m_dwarf, m_states, m_state_map, m_linkage_dependents};
  HPHP::JobQueueDispatcher<Worker> dispatcher{
    num_threads, num_threads, 0, false, &context
  };
  dispatcher.start();

  size_t num_tu = 0;
  FTRACE(1, "Adding type-units to dispatcher...\n");
  // Iterate over every type-unit, enqueuing jobs which will
  // concurrently scan that unit.
  m_dwarf.forEachTopLevelUnit(
    [&] (Dwarf_Die tu) {
      dispatcher.enqueue(m_dwarf.getDIEOffset(tu));
      ++num_tu;
      return true;
    },
    false
  );
  FTRACE(1, "... {} type-units added.\n", num_tu);

  size_t num_cu = 0;
  FTRACE(1, "Adding compilation-units to dispatcher...\n");
  // Iterate over every compilation-unit, enqueuing jobs which will
  // concurrently scan that unit.
  m_dwarf.forEachCompilationUnit(
    [&](Dwarf_Die cu) { dispatcher.enqueue(m_dwarf.getDIEOffset(cu)); ++num_cu;}
  );

  FTRACE(1, "... {} compilation-units added.\n", num_cu);

  // Wait for all the workers to finish.
  dispatcher.stop();

  FTRACE(1, "Finished with genNames\n");

  // If any of the workers caught an exception, rethrow here in the main
  // thread. We don't need to bother taking the lock because all the workers are
  // gone.
  if (context.exception) std::rethrow_exception(context.exception);

  // Since the state map was appended to by the workers in a non-deterministic
  // order, we need to sort it by offset so we can do efficient lookups later.
  std::sort(
    m_state_map.begin(), m_state_map.end(),
    [&](const std::pair<GlobalOff, StateBlock*>& p1,
        const std::pair<GlobalOff, StateBlock*>& p2) {
      return p1.first < p2.first;
    }
  );

  // Some of the static_definitions entries need to be moved to the
  // correct block; eg they were seen when processing the cu
  // containing the definition of the static member, but need to be
  // moved to the state for the tu which contains the definition of
  // the struct (which may or may not be the same state block).
  folly::F14FastSet<void*> seen;
  for (auto const& p : m_state_map) {
    if (!seen.insert(p.second).second) continue;
    auto curOff = p.first;
    auto curState = p.second;
    for (auto it = p.second->static_definitions.begin();
         it != p.second->static_definitions.end(); ) {
      if (it->first != curOff) {
        curOff = it->first;
        curState = const_cast<decltype(p.second)>(&stateForOffset(curOff));
      }
      if (curState == p.second) {
        ++it;
        continue;
      }
      curState->static_definitions.insert(*it);
      it = p.second->static_definitions.erase(it);
    }
  }

  fixTemplateLinkage();
  m_linkage_dependents.clear();
}

size_t TypeParserImpl::getObjectBlockCount() const {
  return m_states.size();
}

const std::vector<ObjectType>&
TypeParserImpl::getObjectBlock(size_t index) const {
  return m_states[index]->all_objs;
}

/*
 * As stated above, the linkage of templates is tricky. The linkage of a
 * template is the most restrictive linkage of its original linkage and the
 * linkage of its template parameters. Since some of the template parameters may
 * not yet be parsed when we parse the template, the inference of the correct
 * template linkage is deferred until all the types' linkages are computed.
 *
 * However, since templates can be parameters to other templates, this process
 * must be repeated until the linkage of no types are changed.
 *
 * As an additional complication, the linkage of any nested class is inherited
 * from its parent, so when a template's linkage changes, it must be bubbled
 * down to any of its nested classes.
 *
 * When the name and initial linkages of all the types was generated, the
 * relationship between templates, their parameters, and nested classes is
 * recorded in linkage_dependents, which is used here.
 */
void TypeParserImpl::fixTemplateLinkage() {
  using ChangedSet = folly::F14FastSet<GlobalOff>;
  ChangedSet changed;

  for (const auto& pair : m_linkage_dependents) {
    if (pair.second.template_uses.empty()) continue;
    changed.emplace(pair.first);
  }

  ChangedSet old_changed;
  while (!changed.empty()) {
    std::swap(changed, old_changed);

    // For every type which has its linkage changed, update its dependents
    // (templates where the type is used as a parameter, or nested classes) with
    // the new linkage, and mark as being changed as well.
    for (auto changed_offset : old_changed) {
      decltype(m_linkage_dependents)::const_accessor acc;
      if (!m_linkage_dependents.find(acc, changed_offset)) continue;

      auto const& children = acc->second.children;
      auto const& template_uses = acc->second.template_uses;

      auto const& changed_state = stateForOffset(changed_offset);

      auto const it = changed_state.obj_offsets.find(changed_offset);
      if (it == changed_state.obj_offsets.end()) {
        // This isn't right - if (eg) its a pointer to an object type
        // with internal linkage, we need to mark the dependents
        // internal; but we don't track pointer types at all - so just
        // assume this type doesn't matter. The same goes for other
        // things like const struct types etc.
        continue;
      }

      auto const& changed_obj = changed_state.all_objs[it->second];

      // Only update and mark if we actually make the linkage more restrictive.
      if (changed_obj.name.linkage != ObjectTypeName::Linkage::external) {
        const auto process = [&](GlobalOff dependent_offset) {
          auto& dep_state = const_cast<StateBlock&>(
            stateForOffset(dependent_offset)
          );
          auto const it = dep_state.obj_offsets.find(dependent_offset);
          if (it == dep_state.obj_offsets.end()) return;
          auto& dependent_obj = dep_state.all_objs[it->second];
          if (dependent_obj.name.linkage < changed_obj.name.linkage) {
            FTRACE(4,
                   "Reducing linkage for {}({}) from {} to {} due to {}({})\n",
                   dependent_obj.name.name,
                   GlobalOff::fromRaw(dependent_obj.key.object_id),
                   show(dependent_obj.name.linkage),
                   show(changed_obj.name.linkage),
                   changed_obj.name.name,
                   GlobalOff::fromRaw(changed_obj.key.object_id));
            dependent_obj.name.linkage = changed_obj.name.linkage;
            changed.emplace(dependent_offset);
          }
        };
        for (auto template_offset : template_uses) process(template_offset);
        for (auto child_offset : children) process(child_offset);
      }
    }

    old_changed.clear();
  }
}

Object TypeParserImpl::getObject(ObjectTypeKey key) {
  auto const& state = stateForOffset(GlobalOff::fromRaw(key.object_id));
  auto iter = state.obj_offsets.find(GlobalOff::fromRaw(key.object_id));
  // If we don't know of an object type at the given location, assume its
  // referring to something we never parsed in the first place, so return the
  // pseudo-type.
  if (iter == state.obj_offsets.end()) {
    return Object{
      ObjectTypeName{
        Scope::s_pseudo_type_name,
        ObjectTypeName::Linkage::pseudo,
      },
      0,
      key,
      Object::Kind::k_other,
      true
    };
  }

  return m_dwarf.onDIEAtOffset(
    GlobalOff::fromRaw(key.object_id),
    [&](Dwarf_Die die) {
      return genObject(
        die,
        state.all_objs[iter->second].name,
        key
      );
    }
  );
}

// For static members, determine how that member's address can be
// determined. In theory, this can be any arbitrary expression, but we only
// support constant addresses right now.
folly::Optional<uintptr_t>
TypeParserImpl::interpretLocAddress(const DwarfState& dwarf,
                                    Dwarf_Attribute attr) {
  auto form = dwarf.getAttributeForm(attr);
  if (form != DW_FORM_exprloc) return folly::none;
  auto exprs = dwarf.getAttributeValueExprLoc(attr);
  if (exprs.size() != 1) return folly::none;
  if (exprs[0].lr_atom != DW_OP_addr) return folly::none;
  return folly::Optional<uintptr_t>{exprs[0].lr_number};
}

folly::Optional<GlobalOff>
TypeParserImpl::parseSpecification(const DwarfState& dwarf,
                                   Dwarf_Die die,
                                   bool first,
                                   StaticSpec &spec) {
  folly::Optional<GlobalOff> offset;
  bool is_inline = false;
  dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (dwarf.getAttributeType(attr)) {
        case DW_AT_abstract_origin:
          offset = dwarf.onDIEAtOffset(
            dwarf.getAttributeValueRef(attr),
            [&](Dwarf_Die die2) {
              return parseSpecification(dwarf, die2, false, spec);
            }
          );
          break;
        case DW_AT_specification:
          offset = dwarf.getAttributeValueRef(attr);
          break;
        case DW_AT_linkage_name:
          if (spec.linkage_name.empty()) {
            spec.linkage_name = dwarf.getAttributeValueString(attr);
          }
          break;
        case DW_AT_location:
          if (spec.address == StaticSpec::kNoAddress) {
            if (auto const address = interpretLocAddress(dwarf, attr)) {
              spec.address = *address;
            }
          }
          break;
        case DW_AT_low_pc:
          if (spec.address == StaticSpec::kNoAddress) {
            spec.address = dwarf.getAttributeValueAddr(attr);
            // Sometimes GCC and Clang will emit invalid function
            // addresses. Usually zero, but sometimes a very low
            // number. These numbers have the appearance of being
            // un-relocated addresses, but its in the final executable. As
            // a safety net, if an address is provided, but its abnormally
            // low, ignore it.
            if (spec.address < 4096) spec.address = StaticSpec::kNoAddress;
          }
          break;
        case DW_AT_object_pointer:
          // Just in case we actually have a definition, use it to infer
          // member-ness.
          spec.is_member = true;
          break;
        default:
          break;
      }
      return true;
    }
  );
  if (first && (is_inline ||
                (spec.linkage_name.empty() &&
                 spec.address == StaticSpec::kNoAddress &&
                 !spec.is_member))) {
    return folly::none;
  }
  return offset;
}

/*
 * Given a DIE, and the current scope, recursively generate the names/linkages
 * for all the object types in this DIE and children. If template_params is
 * provided, the parent DIE is an object type, so template_params should be
 * filled with any template parameters in the child DIE.
 */
void TypeParserImpl::genNames(Env& env,
                              Dwarf_Die die,
                              Scope& scope,
                              std::vector<GlobalOff>* template_params) {
  auto& dwarf = *env.dwarf;
  auto& state = *env.state;

  const auto recurse = [&](std::vector<GlobalOff>* params = nullptr){
    dwarf.forEachChild(
      die,
      [&](Dwarf_Die child) {
        genNames(env, child, scope, params);
        return true;
      }
    );
  };

  auto tag = dwarf.getTag(die);
  switch (tag) {
    case DW_TAG_base_type:
    case DW_TAG_union_type:
    case DW_TAG_enumeration_type:
    case DW_TAG_structure_type:
    case DW_TAG_class_type:
    case DW_TAG_unspecified_type: {
      // Object-types. These have names and linkages, so we must record them.

      // If this is a type-unit definition with a separate declaration
      // in the same tu, declarationOffset will point to the
      // declaration.
      folly::Optional<GlobalOff> declarationOffset;

      // If this is a declaration in a cu, referring back to a
      // tu-definition, definitionOffset will point to that
      // definition. Such declarations are emitted for the
      // *definitions* of static members (which always happen in cus,
      // not tus)
      folly::Optional<GlobalOff> definitionOffset;

      // Determine the base name, whether this type was unnamed, and whether
      // this is an incomplete type or not from the DIE's attributes.
      auto get_info = [&](Dwarf_Die cur,
                          bool updateOffsets) ->
        std::tuple<std::string, bool, bool> {
        std::string name;
        std::string linkage_name;
        auto incomplete = false;

        dwarf.forEachAttribute(
          cur,
          [&](Dwarf_Attribute attr) {
            switch (dwarf.getAttributeType(attr)) {
              case DW_AT_name:
                name = dwarf.getAttributeValueString(attr);
                break;
              case DW_AT_linkage_name:
                linkage_name = dwarf.getAttributeValueString(attr);
                break;
              case DW_AT_declaration:
                incomplete = dwarf.getAttributeValueFlag(attr);
                break;
              case DW_AT_specification:
                // The compiler can spit out a declaration for a
                // struct, followed later by the full definition. The
                // full definition has a DW_AT_specification pointing
                // back to the declaration - but note that the full
                // definition may not be defined in the correct
                // namespace - so we're going to keep the declaration,
                // and update it based on the definition ignoring the
                // definition's name (this feels a little backwards,
                // but its how dwarf works).
                if (updateOffsets) {
                  declarationOffset = dwarf.getAttributeValueRef(attr);
                }
                break;
              case DW_AT_signature:
                if (updateOffsets &&
                    dwarf.getAttributeForm(attr) == DW_FORM_ref_sig8) {
                  // The actual definition is in another type-unit, we
                  // can ignore this declaration.
                  definitionOffset = dwarf.getAttributeValueRef(attr);
                  break;
                }
              default:
                break;
            }
            return true;
          }
        );

        // If there's an explicit name, just use that.
        if (!name.empty()) return std::make_tuple(name, false, incomplete);

        // Otherwise, if there's a linkage name, demangle it, and strip off
        // everything except the last section, and use that as the base
        // name. For types which have external linkage, this lets us use
        // whatever naming scheme the compiler has chosen for unnamed types.
        if (!linkage_name.empty()) {
          auto demangled = folly::demangle(linkage_name.c_str()).toStdString();
          auto index = demangled.rfind("::");
          if (index != decltype(demangled)::npos) demangled.erase(0, index+2);
          return std::make_tuple(demangled, false, incomplete);
        }

        // No explicit name and no linkage name to use, so we have to try to
        // infer one ourself (making it a synthetic name).

        // Try the first named member
        auto const first_member = [&](const char* type,
                                      auto member_type) {
          std::string first_member;
          dwarf.forEachChild(
            cur,
            [&](Dwarf_Die child) {
              if (dwarf.getTag(child) == member_type) {
                first_member = dwarf.getDIEName(child);
              }
              return first_member.empty();
            }
          );
          if (!first_member.empty()) {
            return folly::sformat(
              "(unnamed {} containing '{}')", type, first_member
            );
          }
          return std::string{};
        };

        auto const type_name = [&]{
          if (tag == DW_TAG_enumeration_type) return "enumeration";
          if (tag == DW_TAG_union_type) return "union";
          if (tag == DW_TAG_structure_type) return "struct";
          if (tag == DW_TAG_class_type) return "class";
          return "type";
        };

        auto const member_type = [&]() {
          if (tag == DW_TAG_enumeration_type) return DW_TAG_enumerator;
          return DW_TAG_member;
        };

        auto first_member_name = first_member(type_name(), member_type());
        if (!first_member_name.empty()) {
          return std::make_tuple(
            std::move(first_member_name), true, incomplete
          );
        }

        // If this is within a namespace, don't infer any name at all, keep it
        // nameless. If its not within a namespace (IE, within a class), give it
        // a unique name based on how many unnamed types we've seen so far. We
        // can't do this for types within a namespace because namespaces are
        // open and thus we can't force a global numbering of all types within
        // it.
        if (!scope.isInNamespaceScope()) {
          scope.incUnnamedTypeCount();
          return std::make_tuple(
            folly::sformat(
              "(unnamed {} #{})",
              type_name(),
              scope.unnamedTypeCount()
            ),
            true,
            incomplete
          );
        }

        return std::make_tuple(
          folly::sformat("(unnamed {})", type_name()),
          true,
          incomplete
        );
      };
      const auto info = get_info(die, /*updateOffsets=*/true);

      auto offset = dwarf.getDIEOffset(die);
      if (definitionOffset) {
        // This is a declaration which refers to the definition via
        // DW_AT_signature. We'll see one of these for a class in the
        // cu where its static members are defined.  Later
        // DW_TAG_variable nodes will refer back to the ones here,
        // rather than the ones in the definition, so we need to
        // record a map from any members defined here back to the
        // original definition. We could also see them for parent
        // classes, or for template param (a template param can refer
        // to an out-of-unit type either by using a ref_sig8 directly,
        // in which case we will have resolved the offset correctly,
        // or it could have an offset to a type with a
        // DW_AT_signature, in which case we'll need to fix it up
        // later). In any case, add an entry to map our offset to the
        // true definition, and entries to map any members to their
        // true definitions.
        env.local_mappings.emplace(offset, *definitionOffset);

        folly::F14FastMap<std::string, GlobalOff> map;
        dwarf.forEachChild(
          die,
          [&] (Dwarf_Die child) {
            if (dwarf.getTag(child) == DW_TAG_member) {
              map.emplace(dwarf.getDIEName(child), dwarf.getDIEOffset(child));
            }
            return true;
          }
        );
        if (!map.empty()) {
          dwarf.onDIEAtOffset(
            *definitionOffset,
            [&] (Dwarf_Die orig) {
              dwarf.forEachChild(
                orig,
                [&] (Dwarf_Die child) {
                  auto it = map.find(dwarf.getDIEName(child));
                  if (it != map.end()) {
                    env.local_mappings.emplace(it->second,
                                               dwarf.getDIEOffset(child));
                  }
                  return true;
                }
              );
            }
          );
        }
      }

      auto parent_offset = scope.typeOffset();

      // If we inferred a base name, use that to form the fully qualified name,
      // otherwise treat it as an unnamed type.
      if (!definitionOffset) {
        std::get<1>(info) ?
          scope.pushUnnamedType(std::get<0>(info), offset) :
          scope.pushType(std::get<0>(info), offset);
      } else {
        // Push the name of the definition, not of the declaration
        dwarf.onDIEAtOffset(
            *definitionOffset,
            [&] (Dwarf_Die def) {
              const auto info_def = get_info(def, /*updateOffsets=*/false);
              std::get<1>(info_def) ?
                scope.pushUnnamedType(std::get<0>(info_def), offset) :
                scope.pushType(std::get<0>(info_def), offset);
            });
      }
      SCOPE_EXIT { scope.pop(); };

      if (declarationOffset) {
        // This completes a previous declaration. search backwards for
        // it, which should be fine because its normally right after
        // the declaration (and its always in the same cu/tu).
        auto i = state.all_objs.size();
        while (true) {
          assert(i);
          auto& obj = state.all_objs[--i];
          if (obj.key.object_id == declarationOffset->raw()) {
            assert(obj.incomplete);
            FTRACE(5,
                   "Completing previous definition of {}.\n"
                   "  Was {}, Now {}, Linkage: {}\n",
                   obj.name.name,
                   GlobalOff::fromRaw(obj.key.object_id), offset,
                   show(obj.name.linkage)
                  );
            obj.incomplete = false;
            obj.key.object_id = offset.raw();
            // map declarationOffset to offset, because any ref_sig8s
            // will point to the definition, not the declaration.
            env.local_mappings.emplace(*declarationOffset, offset);

            // Fixup the name in the scope stack
            scope.fixName(obj.name);
            assertx(scope.name().name == obj.name.name);
            break;
          }
        }
      } else {
        // Record this object type, with fully qualified name, key, and linkage.
        auto obj = ObjectType{
          scope.name(),
          ObjectTypeKey{offset.raw(), scope.cuOffset().raw()},
          std::get<2>(info)
        };
        FTRACE(5,
               "{} {} at {} Linkage: {}\n",
               obj.incomplete ? "Declaring" : "Defining",
               obj.name.name,
               offset,
               show(obj.name.linkage)
              );
        state.all_objs.emplace_back(std::move(obj));
      }

      // This object type is done, so recurse into any nested classes. Provide a
      // list of template parameters to be filled in case this is a template. If
      // it is, we'll record the linkage dependence for the later template
      // linkage fix-up.
      std::vector<GlobalOff> recurse_template_params;
      recurse(&recurse_template_params);

      for (auto param_offset : recurse_template_params) {
        FTRACE(9, "linkage: {} depends on template param {}\n",
               offset, param_offset);
        env.linkage_dependents[param_offset].template_uses.emplace(offset);
      }
      if (parent_offset) {
        FTRACE(9, "linkage: {} depends on child {}\n",
               *parent_offset, offset);
        env.linkage_dependents[*parent_offset].children.emplace(offset);
      }
      break;
    }
    case DW_TAG_namespace: {
      // Record the namespace in the scope and recurse. If this is an unnamed
      // namespace, that means any type found in child DIEs will have internal
      // linkage.
      auto name = dwarf.getDIEName(die);
      name.empty() ?
        scope.pushUnnamedNamespace() :
        scope.pushNamespace(std::move(name));
      SCOPE_EXIT { scope.pop(); };
      recurse();
      break;
    }
    case DW_TAG_variable: {
      // Normally we don't care about variables since we're only looking for
      // types. However, certain aspects of object types can't be completely
      // inferred at the declaration site (mainly static variable linkage
      // related things like linkage name and address). We need a definition for
      // that, so record all the variable definitions along with their
      // specification, which we can consult later.

      // Neither GCC nor Clang record a name for a variable which is a static
      // definition, so ignore any that do have a name. This speeds things up.
      if (!dwarf.getDIEName(die).empty()) break;

      StaticSpec spec;
      if (auto off = parseSpecification(dwarf, die, true, spec)) {
        env.raw_static_definitions.emplace_back(*off, spec);
      }
      // Note that we don't recurse into any child DIEs here. There shouldn't be
      // anything interesting in them.
      break;
    }
    case DW_TAG_subprogram: {
      // For the same reason we care about DW_TAG_variables, we examine
      // DW_TAG_subprogram as well. Certain interesting aspects of a static
      // function are only present in its definition.

      if (!dwarf.getDIEName(die).empty()) break;

      StaticSpec spec;
      if (auto off = parseSpecification(dwarf, die, true, spec)) {
        env.raw_static_definitions.emplace_back(*off, spec);
      }

      // Don't recurse. There might be valid types within a subprogram
      // definition, but we deliberately ignore those. A large portion of the
      // debug information lies within subprogram definitions, and scanning all
      // of that consumes a large amount of time. Moreover, these types usually
      // aren't very interesting, so we deliberately ignore them for
      // efficiency. If there's actually any reference to these types, they'll
      // be reported as the pseudo-type.
      break;
    }
    case DW_TAG_template_type_param: {
      // Template type parameters are represented using child DIEs, not
      // attributes. If the parent DIE was an object type, fill the supplied
      // vector with the template parameters. Don't recurse because there
      // shouldn't be anything interesting in the children.
      if (template_params) {
        dwarf.forEachAttribute(
          die,
          [&](Dwarf_Attribute attr) {
            switch (dwarf.getAttributeType(attr)) {
              case DW_AT_type: {
                auto offset = dwarf.getAttributeValueRef(attr);
                // Check this type to see if it is a declaration and use the
                // real type instead
                dwarf.onDIEAtOffset(
                  offset,
                  [&] (Dwarf_Die type_die) {
                    dwarf.forEachAttribute(
                      type_die,
                      [&](Dwarf_Attribute attr) {
                        if (dwarf.getAttributeType(attr) == DW_AT_signature &&
                            dwarf.getAttributeForm(attr) == DW_FORM_ref_sig8) {
                          offset = dwarf.getAttributeValueRef(attr);
                          return false;
                        }
                        return true;
                      }
                    );
                  });
                template_params->emplace_back(offset);
                return false;
              }
              default:
                return true;
            }
          }
        );
      }
      break;
    }
    default:
      recurse();
      break;
  }
}

/*
 * Given the DIE representing an object type, its name, and its key, return the
 * detailed specification of the object.
 */
Object TypeParserImpl::genObject(Dwarf_Die die,
                                 ObjectTypeName name,
                                 ObjectTypeKey key) {
  const auto kind = [&]{
    switch (m_dwarf.getTag(die)) {
      case DW_TAG_structure_type: return Object::Kind::k_class;
      case DW_TAG_class_type: return Object::Kind::k_class;
      case DW_TAG_union_type: return Object::Kind::k_union;
      case DW_TAG_base_type: return Object::Kind::k_primitive;
      case DW_TAG_enumeration_type: return Object::Kind::k_enum;
      // Strange things like "decltype(nullptr_t)"
      case DW_TAG_unspecified_type: return Object::Kind::k_other;
      // Shouldn't happen because we only call genObject() on offsets already
      // visited and verified to be an object type.
      default: always_assert(0);
    }
  }();

  folly::Optional<std::size_t> size;
  bool incomplete = false;
  folly::Optional<GlobalOff> definition_offset;

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_byte_size:
          size = m_dwarf.getAttributeValueUData(attr);
          break;
        case DW_AT_declaration:
          incomplete = m_dwarf.getAttributeValueFlag(attr);
          break;
        case DW_AT_signature:
          definition_offset = m_dwarf.getAttributeValueRef(attr);
          break;
        default:
          break;
      }
      return true;
    }
  );

  if (definition_offset) {
    return m_dwarf.onDIEAtOffset(
      *definition_offset,
      [&](Dwarf_Die die2) { return genObject(die2, name, key); }
    );
  }

  // No size was provided. This is expected for incomplete types or the strange
  // "other" types sometimes seen, but an error otherwise.
  if (!size) {
    if (incomplete || kind == Object::Kind::k_other) {
      size = 0;
    } else {
      throw Exception{
        folly::sformat(
          "Object type '{}' at offset {} is a complete definition, "
          "but has no size!",
          name.name,
          key.object_id
        )
      };
    }
  }

  Object obj{std::move(name), *size, key, kind, incomplete};

  m_dwarf.forEachChild(
    die,
    [&](Dwarf_Die child) {
      switch (m_dwarf.getTag(child)) {
        case DW_TAG_inheritance:
          obj.bases.emplace_back(genBase(child, obj.name));
          break;
        case DW_TAG_member:
          obj.members.emplace_back(genMember(child, obj.name));
          if (obj.name.linkage != ObjectTypeName::Linkage::external) {
            // Clang gives linkage names to things that don't actually have
            // linkage. Don't let any members have linkage names if the object
            // type doesn't have external linkage.
            obj.members.back().linkage_name.clear();
          }
          break;
        case DW_TAG_template_type_parameter:
          obj.template_params.emplace_back(genTemplateParam(child));
          break;
        case DW_TAG_GNU_template_parameter_pack:
          // Flatten parameter packs as if they were just a normally provided
          // parameter list. This is enough for our purposes.
          m_dwarf.forEachChild(
            child,
            [&](Dwarf_Die template_die) {
              if (m_dwarf.getTag(template_die) ==
                  DW_TAG_template_type_parameter) {
                obj.template_params.emplace_back(
                  genTemplateParam(template_die)
                );
              }
              return true;
            }
          );
          break;
        case DW_TAG_subprogram:
          obj.functions.emplace_back(genFunction(child));
          if (obj.name.linkage != ObjectTypeName::Linkage::external) {
            // Clang gives linkage names to things that don't actually have
            // linkage. Don't let any functions have linkage names if the object
            // type doesn't have external linkage.
            obj.functions.back().linkage_name.clear();
          }
          break;
        default:
          break;
      }
      return true;
    }
  );

  // The base classes and members aren't always reported in DWARF in offset
  // order, but make the output deterministic here to simplify consumers of the
  // information.
  std::sort(
    obj.bases.begin(),
    obj.bases.end(),
    [&](const Object::Base& b1, const Object::Base& b2) {
      return std::tie(b1.offset, b1.type.name.name) <
        std::tie(b2.offset, b2.type.name.name);
    }
  );

  std::sort(
    obj.members.begin(),
    obj.members.end(),
    [&](const Object::Member& m1, const Object::Member& m2) {
      return std::tie(m1.offset, m1.name) <
        std::tie(m2.offset, m2.name);
    }
  );

  return obj;
}

/*
 * Given a DIE representing an arbitrary type, return its equivalent Type. This
 * can involve chasing a chain of such type DIEs.
 */
Type TypeParserImpl::genType(Dwarf_Die die) {
  // Offset of a different type this type refers to. If not present, that type
  // is implicitly "void".
  folly::Optional<GlobalOff> type_offset;
  // For pointers to members, the type referring to the object the member
  // belongs to.
  folly::Optional<GlobalOff> containing_type_offset;

  // A struct can have a declaration which refers to the definition
  // via a DW_AT_signature.
  folly::Optional<GlobalOff> definition_offset;

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_type:
          type_offset = m_dwarf.getAttributeValueRef(attr);
          break;
        case DW_AT_containing_type:
          containing_type_offset = m_dwarf.getAttributeValueRef(attr);
          break;
        case DW_AT_signature:
          definition_offset = m_dwarf.getAttributeValueRef(attr);
          return false;
        default:
          break;
      }
      return true;
    }
  );

  const auto recurse = [&](GlobalOff offset) {
    return m_dwarf.onDIEAtOffset(
      offset,
      [&](Dwarf_Die die2) { return genType(die2); }
    );
  };

  // Pointers to member functions aren't represented in DWARF. Instead the
  // compiler creates a struct internally which stores all the information.

  switch (m_dwarf.getTag(die)) {
    case DW_TAG_base_type:
    case DW_TAG_structure_type:
    case DW_TAG_class_type:
    case DW_TAG_union_type:
    case DW_TAG_enumeration_type:
    case DW_TAG_unspecified_type: {
      if (definition_offset) return recurse(*definition_offset);
      auto offset = m_dwarf.getDIEOffset(die);
      auto const& state = stateForOffset(offset);
      auto iter = state.obj_offsets.find(offset);
      if (iter == state.obj_offsets.end()) {
        // Must be the pseudo-type.
        return ObjectType{
          ObjectTypeName{
            Scope::s_pseudo_type_name,
            ObjectTypeName::Linkage::pseudo
          },
          ObjectTypeKey{offset.raw(), 0},
          true
        };
      } else {
        return state.all_objs[iter->second];
      }
    }
    case DW_TAG_pointer_type:
      return PtrType{type_offset ? recurse(*type_offset) : VoidType{}};
    case DW_TAG_reference_type: {
      if (!type_offset) {
        throw Exception{
          folly::sformat(
            "Encountered reference to void at offset {}",
            m_dwarf.getDIEOffset(die)
          )
        };
      }
      return RefType{recurse(*type_offset)};
    }
    case DW_TAG_rvalue_reference_type: {
      if (!type_offset) {
        throw Exception{
          folly::sformat(
            "Encountered rvalue reference to void at offset {}",
            m_dwarf.getDIEOffset(die)
          )
        };
      }
      return RValueRefType{recurse(*type_offset)};
    }
    case DW_TAG_array_type: {
      if (!type_offset) {
        throw Exception{
          folly::sformat(
            "Encountered array of voids at offset {}",
            m_dwarf.getDIEOffset(die)
          )
        };
      }
      return ArrType{recurse(*type_offset), determineArrayBound(die)};
    }
    case DW_TAG_const_type:
      return ConstType{type_offset ? recurse(*type_offset) : VoidType{}};
    case DW_TAG_volatile_type:
      return VolatileType{type_offset ? recurse(*type_offset) : VoidType{}};
    case DW_TAG_restrict_type:
      return RestrictType{type_offset ? recurse(*type_offset) : VoidType{}};
    case DW_TAG_typedef:
      return type_offset ? recurse(*type_offset) : VoidType{};
    case DW_TAG_subroutine_type: {
      FuncType func{type_offset ? recurse(*type_offset) : VoidType{}};
      fillFuncArgs(die, func);
      return std::move(func);
    }
    case DW_TAG_ptr_to_member_type: {
      if (!containing_type_offset) {
        throw Exception{
          folly::sformat(
            "Encountered ptr-to-member at offset {} without a "
            "containing object",
            m_dwarf.getDIEOffset(die)
          )
        };
      }

      auto containing = recurse(*containing_type_offset);
      if (auto obj = containing.asObject()) {
        return PtrType{
          MemberType{std::move(*obj), recurse(*type_offset)}
        };
      } else {
        throw Exception{
          folly::sformat(
            "Encountered ptr-to-member at offset {} with a "
            "containing object of type '{}'",
            m_dwarf.getDIEOffset(die),
            containing.toString()
          )
        };
      }
    }
    default:
      throw Exception{
        folly::sformat(
          "Encountered non-type tag '{}' at offset {} while "
          "traversing type description",
          m_dwarf.tagToString(m_dwarf.getTag(die)),
          m_dwarf.getDIEOffset(die)
        )
      };
  }
}

Object::Member TypeParserImpl::genMember(Dwarf_Die die,
                                         const ObjectTypeName& parent_name) {
  std::string name;
  std::string linkage_name;
  std::size_t offset = 0;
  folly::Optional<GlobalOff> die_offset;
  folly::Optional<uintptr_t> address;
  bool is_static = false;

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_name:
          name = m_dwarf.getAttributeValueString(attr);
          break;
        case DW_AT_linkage_name:
          linkage_name = m_dwarf.getAttributeValueString(attr);
          break;
        case DW_AT_location:
          address = interpretLocAddress(m_dwarf, attr);
          break;
        case DW_AT_data_member_location:
          offset = m_dwarf.getAttributeValueUData(attr);
          break;
        case DW_AT_type:
          die_offset = m_dwarf.getAttributeValueRef(attr);
          break;
        case DW_AT_declaration:
          is_static = m_dwarf.getAttributeValueFlag(attr);
          break;
        default:
          break;
      }
      return true;
    }
  );

  if (!die_offset) {
    // No DW_AT_type means "void", but you can't have void members!
    throw Exception{
      folly::sformat(
        "Encountered member (name: '{}') of type void "
        "in object type '{}' at offset {}",
        name,
        parent_name.name,
        m_dwarf.getDIEOffset(die)
      )
    };
  }

  if (is_static) {
    // If this is a static member, look up any definitions which refer to this
    // member, and pull any additional information out of it.
    auto const static_offset = m_dwarf.getDIEOffset(die);
    auto const& state = stateForOffset(static_offset);
    auto const range = state.static_definitions.equal_range(static_offset);

    for (auto const& elm : range) {
      if (linkage_name.empty() && !elm.second.linkage_name.empty()) {
        linkage_name = elm.second.linkage_name;
      }
      if (!address && elm.second.address != StaticSpec::kNoAddress) {
        address = elm.second.address;
      }
    }
  }

  auto type = m_dwarf.onDIEAtOffset(
    *die_offset,
    [&](Dwarf_Die die2){ return genType(die2); }
  );

  if (name.empty()) {
    name = is_static
      ? folly::sformat("(unnamed static member of type '{}')", type.toString())
      : folly::sformat("(unnamed member of type '{}')", type.toString());
  }

  return Object::Member{
    name,
    is_static ? folly::none : folly::Optional<std::size_t>{offset},
    linkage_name,
    address,
    std::move(type)
  };
}

Object::Function TypeParserImpl::genFunction(Dwarf_Die die) {
  std::string name;
  Type ret_type{VoidType{}};
  std::string linkage_name;
  bool is_virtual = false;
  bool is_member = false;

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_name:
          name = m_dwarf.getAttributeValueString(attr);
          break;
        case DW_AT_type:
          ret_type = m_dwarf.onDIEAtOffset(
            m_dwarf.getAttributeValueRef(attr),
            [&](Dwarf_Die ty_die) { return genType(ty_die); }
          );
          break;
        case DW_AT_linkage_name:
          linkage_name = m_dwarf.getAttributeValueString(attr);
          break;
        case DW_AT_virtuality:
          is_virtual =
            (m_dwarf.getAttributeValueUData(attr) != DW_VIRTUALITY_none);
          break;
        case DW_AT_object_pointer:
          is_member = true;
          break;
        default:
          break;
      }
      return true;
    }
  );

  /*
   * We need to determine if this function is a static function or a member
   * function. The straight-forward way is to look for the DW_AT_object_pointer
   * attribute (which is only present for member functions). This works fine for
   * GCC, but not Clang.
   *
   * On Clang, the DW_AT_object_pointer is only present in a function's
   * definition, not its declaration. Moreover, it doesn't reliably emit
   * function declarations if it thinks the function isn't used. As a result, we
   * can't reliably distinguish member functions from static functions on clang.
   *
   * As an alternative, if the first formal parameter of a function is marked as
   * being "artificial" (which means its not present in the actual source),
   * assume its actually the this pointer, and that the function is a member
   * function.
   */
  std::vector<Type> arg_types;
  m_dwarf.forEachChild(
    die,
    [&](Dwarf_Die child) {
      if (m_dwarf.getTag(child) != DW_TAG_formal_parameter) {
        return true;
      }

      bool is_artificial = false;
      Type arg_type{VoidType()};

      m_dwarf.forEachAttribute(
        child,
        [&](Dwarf_Attribute attr) {
          switch (m_dwarf.getAttributeType(attr)) {
            case DW_AT_type:
              arg_type = m_dwarf.onDIEAtOffset(
                m_dwarf.getAttributeValueRef(attr),
                [&](Dwarf_Die ty_die) { return genType(ty_die); }
              );
              break;
            case DW_AT_artificial:
              is_artificial = m_dwarf.getAttributeValueFlag(attr);
              break;
            default:
              break;
          }
          return true;
        }
      );

      // Only consider this a member function if this arg if the first and its
      // artificial.
      if (is_artificial && arg_types.empty()) {
        is_member = true;
      }
      arg_types.emplace_back(std::move(arg_type));

      return true;
    }
  );

  folly::Optional<std::uintptr_t> address;

  // Similar to static variables, find any definitions which refer to this
  // function in order to extract linkage information.
  auto const offset = m_dwarf.getDIEOffset(die);
  auto const& state = stateForOffset(offset);
  auto range = state.static_definitions.equal_range(offset);
  for (auto const& elm : range) {
    if (linkage_name.empty() && !elm.second.linkage_name.empty()) {
      linkage_name = elm.second.linkage_name;
    }
    if (!address && elm.second.address != StaticSpec::kNoAddress) {
      address = elm.second.address;
    }
    if (elm.second.is_member) is_member = true;
  }

  return Object::Function{
    name,
    std::move(ret_type),
    std::move(arg_types),
    is_virtual ?
      Object::Function::Kind::k_virtual :
      (is_member ? Object::Function::Kind::k_member :
       Object::Function::Kind::k_static),
    linkage_name,
    address,
  };
}

Object::Base TypeParserImpl::genBase(Dwarf_Die die,
                                     const ObjectTypeName& parent_name) {
  std::string name;
  folly::Optional<std::size_t> offset;
  folly::Optional<GlobalOff> die_offset;
  bool is_virtual = false;

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_name:
          name = m_dwarf.getAttributeValueString(attr);
          break;
        case DW_AT_type:
          die_offset = m_dwarf.getAttributeValueRef(attr);
          break;
        case DW_AT_virtuality:
          is_virtual =
            (m_dwarf.getAttributeValueUData(attr) != DW_VIRTUALITY_none);
          break;
        default:
          break;
      }
      return true;
    }
  );

  if (!is_virtual) {
    offset = 0;

    m_dwarf.forEachAttribute(
      die,
      [&](Dwarf_Attribute attr) {
        switch (m_dwarf.getAttributeType(attr)) {
          case DW_AT_data_member_location:
            offset = m_dwarf.getAttributeValueUData(attr);
            break;
          default:
            break;
        }
        return true;
      }
    );
  }

  if (!die_offset) {
    throw Exception{
      folly::sformat(
        "Encountered base '{}' of object type '{}' without "
        "type information at offset {}",
        name,
        parent_name.name,
        m_dwarf.getDIEOffset(die)
      )
    };
  }

  auto type =
    m_dwarf.onDIEAtOffset(
      *die_offset,
      [&](Dwarf_Die die2) { return genType(die2); }
    );

  if (auto obj = type.asObject()) {
    // Base class better be an actual class!
    return Object::Base{*obj, offset};
  } else {
    throw Exception{
      folly::sformat(
        "Encountered base '{}' of object type '{}' of "
        "non-object type '{}' at offset {}",
        name,
        parent_name.name,
        type.toString(),
        m_dwarf.getDIEOffset(die)
      )
    };
  }
}

Object::TemplateParam TypeParserImpl::genTemplateParam(Dwarf_Die die) {
  folly::Optional<GlobalOff> die_offset;

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_type:
          die_offset = m_dwarf.getAttributeValueRef(attr);
          break;
        default:
          break;
      }
      return true;
    }
  );

  return Object::TemplateParam{
    die_offset ?
      m_dwarf.onDIEAtOffset(
        *die_offset,
        [&](Dwarf_Die die2){ return genType(die2); }
      ) :
      VoidType{}
  };
}

folly::Optional<std::size_t>
TypeParserImpl::determineArrayBound(Dwarf_Die die) {
  folly::Optional<std::size_t> bound;

  m_dwarf.forEachChild(
    die,
    [&](Dwarf_Die child) {
      switch (m_dwarf.getTag(child)) {
        case DW_TAG_subrange_type:
          m_dwarf.forEachAttribute(
            child,
            [&](Dwarf_Attribute attr) {
              switch (m_dwarf.getAttributeType(attr)) {
                case DW_AT_count:
                  bound = m_dwarf.getAttributeValueUData(attr);
                  break;
                case DW_AT_upper_bound:
                  bound = m_dwarf.getAttributeValueUData(attr)+1;
                  break;
                default:
                  break;
              }
              return true;
            }
          );
          break;
        default:
          break;
      }
      return true;
    }
  );

  if (bound && !*bound) bound.reset();
  return bound;
}

void TypeParserImpl::fillFuncArgs(Dwarf_Die die, FuncType& func) {
  m_dwarf.forEachChild(
    die,
    [&](Dwarf_Die child) {
      switch (m_dwarf.getTag(child)) {
        case DW_TAG_formal_parameter: {
          folly::Optional<GlobalOff> type_offset;

          m_dwarf.forEachAttribute(
            child,
            [&](Dwarf_Attribute attr) {
              switch (m_dwarf.getAttributeType(attr)) {
                case DW_AT_type:
                  type_offset = m_dwarf.getAttributeValueRef(attr);
                  break;
                default:
                  break;
              }
              return true;
            }
          );

          if (!type_offset) {
            throw Exception{
              folly::sformat(
                "Encountered function at offset {} taking a void parameter",
                m_dwarf.getDIEOffset(die)
              )
            };
          }

          func.args.push_back(
            m_dwarf.onDIEAtOffset(
              *type_offset,
              [&](Dwarf_Die die) { return genType(die); }
            )
          );
          break;
        }
        default:
          break;
      }
      return true;
    }
  );
}

/*
 * Print out the given DIE (including children) in textual format to the given
 * ostream. Only actually print out DIEs which begin in the range between the
 * begin and end parameters.
 */

void printDIE(std::ostream& os,
              const DwarfState& dwarf,
              Dwarf_Die die,
              std::pair<uint64_t,GlobalOff>* sig,
              std::size_t begin,
              std::size_t end,
              int indent = 0) {
  auto tag = dwarf.getTag(die);
  auto tag_name = dwarf.tagToString(tag);
  auto name = dwarf.getDIEName(die);
  auto offset = dwarf.getDIEOffset(die).offset();

  const auto recurse = [&]{
    // Find the last child DIE which does not start with the begin/end
    // range. This DIE is the first one which contains some data within the
    // begin/end range, so that must be the first one to begin recursion at.
    folly::Optional<uint64_t> first;
    if (begin > 0) {
      dwarf.forEachChild(
        die,
        [&](Dwarf_Die child) {
          const auto offset = dwarf.getDIEOffset(child).offset();
          if (offset <= begin) {
            first = offset;
            return true;
          } else {
            return false;
          }
        }
      );
    }

    // Only actually recurse if this child DIE is the above computed first DIE,
    // or one following it, and begins before the end parameter.
    dwarf.forEachChild(
      die,
      [&](Dwarf_Die child) {
        const auto offset = dwarf.getDIEOffset(child).offset();
        if ((!first || offset >= *first) && offset < end) {
          printDIE(os, dwarf, child, nullptr, begin, end, indent+1);
        }
        return offset < end;
      }
    );
  };

  if (offset < begin) {
    recurse();
    return;
  } else if (offset >= end) {
    return;
  }

  auto const printSig = [&] (uint64_t sig) {
    return folly::sformat("ref_sig8:{:016x}", sig);
  };

  for (int i = 0; i < indent; ++i) {
    os << "  ";
  }
  os << "#" << offset << ": " << tag_name << " (" << tag << ") \""
     << name << "\"";
  if (sig && sig->first) {
    os << folly::sformat(" {{{} -> #{}}}", printSig(sig->first), sig->second);
  }
  os << "\n";

  dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      auto const type = dwarf.getAttributeType(attr);
      auto const attr_name = dwarf.attributeTypeToString(type);
      auto const form = dwarf.getAttributeForm(attr);
      auto const attr_form = dwarf.attributeFormToString(form);

      auto attr_value = [&]() -> std::string {
        if (type == DW_AT_ranges) {
          auto const ranges = dwarf.getRanges(attr);
          std::string res;
          for (auto range : ranges) {
            if (range.dwr_addr1 == DwarfState::Dwarf_Ranges::kSelection) {
              folly::format(&res, "0x{:x} ", range.dwr_addr2);
            } else {
              folly::format(&res, "0x{:x}-0x{:x} ",
                            range.dwr_addr1, range.dwr_addr2);
            }
          }
          return res;
        }
        switch (dwarf.getAttributeForm(attr)) {
          case DW_FORM_data1:
          case DW_FORM_data2:
          case DW_FORM_data4:
          case DW_FORM_data8:
          case DW_FORM_udata:
            return folly::sformat("{}", dwarf.getAttributeValueUData(attr));

          case DW_FORM_sdata:
            return folly::sformat("{}", dwarf.getAttributeValueSData(attr));

          case DW_FORM_string:
          case DW_FORM_strp:
            return folly::sformat(
              "\"{}\"",
              dwarf.getAttributeValueString(attr)
            );

          case DW_FORM_flag:
          case DW_FORM_flag_present:
            return dwarf.getAttributeValueFlag(attr) ? "true" : "false";

          case DW_FORM_addr:
            return folly::sformat(
              "{:#010x}",
              dwarf.getAttributeValueAddr(attr)
            );

          case DW_FORM_ref1:
          case DW_FORM_ref2:
          case DW_FORM_ref4:
          case DW_FORM_ref8:
          case DW_FORM_ref_udata:
          case DW_FORM_ref_addr:
            return folly::sformat("#{}", dwarf.getAttributeValueRef(attr));
          case DW_FORM_ref_sig8: {
            return printSig(dwarf.getAttributeValueSig8(attr));
          }

          case DW_FORM_exprloc: {
            std::string output;
            for (const auto& expr : dwarf.getAttributeValueExprLoc(attr)) {
              if (expr.lr_atom == DW_OP_addr) {
                output += folly::sformat(
                  "<OP_addr: {:#x}>,",
                  expr.lr_number
                );
              } else {
                output += folly::sformat(
                  "<{}:{}:{}:{}>,",
                  dwarf.opToString(expr.lr_atom),
                  expr.lr_number,
                  expr.lr_number2,
                  expr.lr_offset
                );
              }
            }
            return folly::sformat("Location: [{}]", output);
          }

          case DW_FORM_block1:
          case DW_FORM_block2:
          case DW_FORM_block4:
          case DW_FORM_block: return "{BLOCK}";

          case DW_FORM_indirect: return "{INDIRECT}";
          case DW_FORM_sec_offset: return "{SECTION OFFSET}";
          default: return "{UNKNOWN}";
        }
      }();

      for (int i = 0; i < indent; ++i) {
        os << "  ";
      }
      os << folly::sformat("   **** {} ({}) ==> {} [{}:{}]\n",
                           attr_name, type, attr_value,
                           attr_form, form);
      return true;
    }
  );

  recurse();
}

struct PrinterImpl : Printer {
  explicit PrinterImpl(const std::string& filename): m_filename{filename} {}
  void operator()(std::ostream& os,
                  std::size_t begin,
                  std::size_t end) const override {
    DwarfState dwarf{m_filename};

    print_section(os, dwarf, false, begin, end);
    print_section(os, dwarf, true, begin, end);

    os << std::flush;
  }
private:
  void print_section(std::ostream& os,
                     const DwarfState& dwarf,
                     bool isInfo,
                     std::size_t begin,
                     std::size_t end) const {
    // If a non-default begin parameter was specified, first iterate over all
    // the compilation units. Find the first compilation unit which at least
    // partially lies within the range given by the begin parameter. This is the
    // first compilation unit to begin printing from.
    folly::Optional<uint64_t> last;
    if (begin > 0) {
      dwarf.forEachTopLevelUnit(
        [&](Dwarf_Die cu) {
          const auto offset = dwarf.getDIEOffset(cu).offset();
          if (offset <= begin) last = offset;
        },
        isInfo
      );
    }

    // Now iterate over all the compilation units again. Only actually print out
    // compilation units if they lie within the begin/end parameter range.
    dwarf.forEachTopLevelUnit(
      [&] (Dwarf_Die cu) {
        auto context = cu->context;
        auto type_offset = GlobalOff { context->typeOffset, context->isInfo };
        auto pair = std::make_pair(context->typeSignature, type_offset);
        const auto offset = dwarf.getDIEOffset(cu).offset();
        if (offset >= end) return false;
        if ((!last || offset >= *last)) {
          printDIE(
            os,
            dwarf,
            cu,
            &pair,
            // If this compilation unit entirely lies within the begin/end
            // range, specify a begin parameter of "0", which will stop
            // printDIE() from doing range checks (which is more efficient).
            (!last || (offset > *last)) ? 0 : begin,
            end
          );
        }
        return true;
      },
      isInfo
    );
  }
  std::string m_filename;
};


struct GDBIndexerImpl : GDBIndexer {
explicit GDBIndexerImpl(const std::string& filename, int num_threads)
  : m_filename{filename}
  , m_numThreads{num_threads}
  {
    if (num_threads < 1) {
      throw Exception{folly::sformat("Invalid number of threads: {}",
                                     num_threads)};
    }
  }

  void operator()(const std::string& output_file) const override {
    auto begin_time = ::HPHP::Timer::GetCurrentTimeMicros();
    DwarfState dwarf{m_filename};
    log_time(begin_time, "Parsing dwarf file");

    std::FILE* fd = std::fopen(output_file.c_str(), "wb");

    if (!fd) {
      throw Exception{folly::sformat("Cannot open file: {}", output_file)};
    }

    auto const gdb_index_version = 8;
    std::vector<uint32_t> header{gdb_index_version, 0, 0, 0, 0, 0};

    auto time_index_begin = ::HPHP::Timer::GetCurrentTimeMicros();

    auto addresses_and_symbols = collect_addresses_and_symbols(dwarf);
    auto time = log_time(time_index_begin, "collect_addresses_and_symbols");
    auto const cu = get_cu(dwarf);
    time = log_time(time, "Get_cu");
    auto const tu = get_tu(dwarf);
    time = log_time(time, "Get_tu");
    auto const address = get_address(addresses_and_symbols.first);
    time = log_time(time, "Get_address");
    auto const  symbol_and_constants =
      get_symbol_and_constants(addresses_and_symbols.second);
    log_time(time, "Get_symbol_and_constants");

    time = log_time(time_index_begin, "Index generation");

    // The offset, from the start of the file, of the CU list.
    header[1] = sizeof header[0] * header.size();
    // The offset, from the start of the file, of the types CU list.
    header[2] = header[1] + sizeof cu[0] * cu.size();
    // The offset, from the start of the file, of the address area.
    header[3] = header[2] + sizeof tu[0] * tu.size();
    // The offset, from the start of the file, of the symbol table.
    header[4] = header[3] + sizeof address[0] * address.size();
    // The offset, from the start of the file, of the constant pool.
    header[5] = header[4] +
      sizeof symbol_and_constants.symbol_pool.m_hashtable[0] *
      symbol_and_constants.symbol_pool.m_hashtable.size();

    print_section(fd, header);
    print_section(fd, cu);
    print_section(fd, tu);
    print_section(fd, address);
    print_section(fd, symbol_and_constants.symbol_pool.m_hashtable);
    print_section(fd, symbol_and_constants.cu_vector_offsets);
    print_section(fd, symbol_and_constants.strings);

    log_time(time, "Print");

    log_time(begin_time, "Full index creation");

    std::fclose(fd);
  }

private:
  int32_t log_time(int32_t time, const char* msg) const {
    int32_t now = ::HPHP::Timer::GetCurrentTimeMicros();
    std::cout << msg << " took " << (now - time) / 1000 << " ms" << std::endl;
    return now;
  }

  void print_section(std::FILE* fd,
                     const std::vector<std::string>& data) const {
    if (!data.size()) return;
    assertx(fd);
    for (auto s : data) {
      std::fwrite(s.c_str(), sizeof(char), s.length() + 1, fd);
    }
  }

  template <typename T>
  void print_section(std::FILE* fd, const std::vector<T>& data) const {
    if (!data.size()) return;
    assertx(fd);
    std::fwrite(data.data(), sizeof data[0], data.size(), fd);
  }

  std::vector<uint64_t> get_cu(const DwarfState& dwarf) const {
    std::vector<uint64_t> result = {};
    dwarf.forEachCompilationUnit(
      [&](Dwarf_Die cu) {
        result.push_back(cu->context->offset);
        result.push_back(cu->context->size);
      }
    );
    return result;
  }

  std::vector<uint64_t> get_tu(const DwarfState& dwarf) const {
    std::vector<uint64_t> result = {};
    dwarf.forEachTopLevelUnit(
      [&](Dwarf_Die cu) {
        result.push_back(cu->context->offset);
        result.push_back(cu->context->typeOffset - cu->context->offset);
        result.push_back(cu->context->typeSignature);
      }, false
    );
    return result;
  }

  struct AddressTableEntry {
    union {
      uint64_t low;
      struct {
        uint32_t low_bottom;
        uint32_t low_top;
      };
    };
    union {
      uint64_t high;
      struct {
        uint32_t high_bottom;
        uint32_t high_top;
      };
    };
    uint32_t index;
  };

  static bool compareAddressTableEntry(AddressTableEntry a,
                                       AddressTableEntry b) {
    return a.low == b.low ? a.high < b.high : a.low < b.low;
  }

  void visit_die_for_address(const DwarfState& dwarf, const Dwarf_Die die,
                             std::vector<AddressTableEntry>& entries,
                             uint32_t cu_index) const {
    folly::Optional<uint64_t> low, high;
    std::vector<DwarfState::Dwarf_Ranges> ranges;
    bool is_high_udata = false;
    dwarf.forEachAttribute(
      die,
      [&](Dwarf_Attribute attr) {
        switch (dwarf.getAttributeType(attr)) {
          case DW_AT_ranges:
            ranges = dwarf.getRanges(attr);
            break;
          case DW_AT_low_pc:
            // Some times GCC/Clang emits very low numbers for addresses in
            // the form of UData. Let's drop them.
            if (attr->form == DW_FORM_addr) {
              low = dwarf.getAttributeValueAddr(attr);
            }
            break;
          case DW_AT_high_pc:
            if (attr->form != DW_FORM_addr) {
              is_high_udata = true;
              high = dwarf.getAttributeValueUData(attr);
            } else {
              high = dwarf.getAttributeValueAddr(attr);
            }
            break;
          default:
            break;
        }
        return true;
      }
    );

    if (!ranges.empty()) {
      uint64_t base = low ? *low : 0;
      bool added = false;
      for (auto range : ranges) {
        if (range.dwr_addr1 == DwarfState::Dwarf_Ranges::kSelection) {
          base = range.dwr_addr2;
          continue;
        }
        if (base + range.dwr_addr1 == 0) continue;
        // Drop all the addresses under 2M
        if (base + range.dwr_addr2 < 2000000) continue;
        added = true;
        entries.push_back(
          AddressTableEntry {
            base + range.dwr_addr1,
            base + range.dwr_addr2,
            cu_index
          }
        );
      }
      if (added) return;
    }

    if (low && high) {
      high = is_high_udata ? *low + *high : *high;
      // Drop all the addresses under 2M
      if (*low != 0 && *high >= 2000000) {
        entries.push_back(AddressTableEntry{*low, *high, cu_index});
        return;
      }
    }

    dwarf.forEachChild(
      die,
      [&](Dwarf_Die child) {
        visit_die_for_address(dwarf, child, entries, cu_index);
        return true;
      }
    );
  }

  std::vector<uint32_t>
  get_address(std::vector<AddressTableEntry>& entries) const {
    sort(entries.begin(), entries.end(), compareAddressTableEntry);

    // Split into little-endian formatting
    std::vector<uint32_t> result = {};
    for (auto& e : entries) {
      result.push_back(e.low_bottom);
      result.push_back(e.low_top);
      result.push_back(e.high_bottom);
      result.push_back(e.high_top);
      result.push_back(e.index);
    }
    return result;
  }

  struct GDBSymbol {
    uint32_t name_offset{};
    uint32_t cu_vector_offset{};

    bool valid() { return name_offset; }
  };

  struct GDBHashtable {
    GDBHashtable() : m_size(0), m_capacity(0), m_hashtable({}) {}
    size_t m_size;
    size_t m_capacity;
    std::vector<GDBSymbol> m_hashtable;


    void init(size_t size) {
      assertx(m_size == 0 && m_capacity == 0);

      auto const nextPowerOfTwo = [](size_t n) -> size_t {
        if (n == 0) return 1;
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
        return n;
      };

      auto initial_size = nextPowerOfTwo(size * 4 / 3);

      m_hashtable = std::vector<GDBSymbol>(initial_size, GDBSymbol{});
      m_capacity = initial_size;
    }

    GDBSymbol* findSlot(uint32_t hash) {
      uint32_t index = hash;
      uint32_t step = ((hash * 17) & (m_capacity - 1)) | 1;

      while (true) {
        index &= m_capacity - 1;
        if (!m_hashtable[index].valid()) {
          return &m_hashtable[index];
        }
        index += step;
      }
    }

    bool add(uint32_t hash, GDBSymbol s) {
      auto const loc = this->findSlot(hash);
      assert(!loc->valid());
      *loc = s;
      m_size++;
      return true;
    }
  };

  using SymbolMap = tbb::concurrent_hash_map<std::string,
                                             std::vector<uint32_t>,
                                             ::HPHP::stringHashCompare>;
  using SpecMap = folly::F14FastMap<GlobalOff, std::string>;

  void visit_die_for_symbols(const DwarfState& dwarf,
                             const Dwarf_Die die,
                             SymbolMap& symbols,
                             SpecMap& spec_names,
                             std::string parent_name,
                             uint32_t language,
                             uint32_t cu_index) const {

    bool is_declaration = false;
    bool is_external = false;
    std::string name;
    bool full_name = false;
    bool is_inlined = false;
    bool has_location = false;
    bool in_specification = false;
    auto specification = GlobalOff::fromRaw(0);
    auto collect_attributes = [&] (Dwarf_Attribute attr) {
      switch (dwarf.getAttributeType(attr)) {
        case DW_AT_declaration:
          if (!in_specification) {
            is_declaration = dwarf.getAttributeValueFlag(attr);
          }
          break;
        case DW_AT_external:
          is_external = dwarf.getAttributeValueFlag(attr);
          break;
        case DW_AT_linkage_name:
          is_external = true;
          break;
        case DW_AT_location:
          has_location = true;
          break;
        case DW_AT_name:
          if (!full_name) {
            name = dwarf.getAttributeValueString(attr);
          }
          break;
        case DW_AT_inline: {
          auto const val = dwarf.getAttributeValueUData(attr);
          is_inlined =
            (val == DW_INL_inlined) ||
            (val == DW_INL_declared_inlined);
          break;
        }
        case DW_AT_language:
          language = dwarf.getAttributeValueUData(attr);
          break;
        case DW_AT_specification: {
          specification = dwarf.getAttributeValueRef(attr);
          auto const it = spec_names.find(specification);
          if (it != spec_names.end()) {
            name = it->second;
            auto const pos = name.rfind("::");
            if (pos != std::string::npos) {
              parent_name = name.substr(0, pos);
            }
            full_name = true;
          }
          break;
        }
        default:
          return true;
      }
      return true;
    };
    dwarf.forEachAttribute(die, collect_attributes);
    if (specification.raw()) {
      dwarf.onDIEAtOffset(
        specification,
        [&] (Dwarf_Die d) {
          in_specification = true;
          dwarf.forEachAttribute(d, collect_attributes);
        }
      );
    }

    struct IndexAndFlags {
      IndexAndFlags(uint32_t index, uint32_t kind, uint32_t is_static) {
        assertx((index >> 24) == 0);
        // Bits 0-23 is CU index
        // Bits 24-27 are reserved and must be 0
        // Bits 28-30 The kind of the symbol in the CU.
        // Bit 31 is zero if the value is global and one if it is static.
        m_data = (is_static << 31) | (kind << 28) | index;
      }

      explicit IndexAndFlags(uint32_t data) : m_data(data) {}

      uint32_t m_data;

      uint32_t get_kind()      const { return (m_data >> 28) & 7; }
      uint32_t get_is_static() const { return m_data >> 31; }
    };


    constexpr int TYPE = 1;
    constexpr int VARIABLE = 2;
    //constexpr int ENUM = 2;
    constexpr int FUNCTION = 3;
    // constexpr int OTHER = 4;

    auto const index_and_flags = [&] {
      uint32_t kind = 0;
      auto is_static = false;
      switch (dwarf.getTag(die)) {
        case DW_TAG_typedef:
        case DW_TAG_base_type:
        case DW_TAG_subrange_type:
          kind = TYPE;
          is_static = 1;
          break;
        case DW_TAG_enumerator:
          kind = VARIABLE;
          is_static = language != DW_LANG_C_plus_plus;
          break;
        case DW_TAG_subprogram:
          kind = FUNCTION;
          is_static = !(is_external || language == DW_LANG_Ada83 ||
                        language == DW_LANG_Ada95);
          break;
        case DW_TAG_constant:
          kind = VARIABLE;
          is_static = !is_external;
          break;
        case DW_TAG_variable:
          kind = VARIABLE;
          is_static = !is_external;
          break;
        case DW_TAG_namespace:
          kind = TYPE;
          is_static = 0;
          break;
        case DW_TAG_class_type:
        case DW_TAG_interface_type:
        case DW_TAG_structure_type:
        case DW_TAG_union_type:
        case DW_TAG_enumeration_type:
          kind = TYPE;
          is_static = language != DW_LANG_C_plus_plus;
          break;
        default:
          throw Exception{"Invalid tag"};
      }
      return IndexAndFlags{cu_index, kind, is_static}.m_data;
    };

    auto const hasSameFlags = [&](std::vector<uint32_t> v, uint32_t input) {
      auto const flags = IndexAndFlags{input};
      for (auto const e : v) {
        auto const f = IndexAndFlags{e};
        if (f.get_kind() == flags.get_kind()) {
          if ((f.get_kind() == TYPE &&
               f.get_is_static() == flags.get_is_static()) ||
              (!f.get_is_static() && !flags.get_is_static())) {
            return true;
          }
        }
      }
      return false;
    };

    auto const addSymbol = [&](std::string name) {
      auto value = index_and_flags();
      SymbolMap::accessor acc;
      if (symbols.insert(acc, name) || !hasSameFlags(acc->second, value)) {
        acc->second.push_back(value);
      }
    };

    auto const addParent = [&] {
      if (full_name) return;
      if (name.empty()) return;
      if (!parent_name.empty()) {
        name = folly::sformat("{}::{}", parent_name, name);
      }
      if (is_declaration) {
        spec_names.emplace(dwarf.getDIEOffset(die), name);
      }
    };

    auto const visitChildren = [&](std::string name) {
      dwarf.forEachChild(
        die,
        [&](Dwarf_Die child) {
          visit_die_for_symbols(dwarf, child, symbols, spec_names, name,
                                language, cu_index);
          return true;
        }
      );
    };

    auto const tag = dwarf.getTag(die);
    switch (tag) {
      case DW_TAG_base_type:
        // don't canonicalize!
        addSymbol(name);
        break;
      case DW_TAG_member:
        // static members appear first here as a declaration, then
        // later as a DW_TAG_variable whose specification points
        // here. We need to note the name just in case.
        if (is_declaration) addParent();
        break;
      case DW_TAG_subprogram:
        if (is_inlined) break;
      case DW_TAG_constant:
      case DW_TAG_enumerator:
        if (name.empty()) break;
        addParent();
        if (is_declaration) break;
        addSymbol(name);
        break;
      case DW_TAG_variable:
        if (name.empty() || (!is_external && !has_location)) break;
        addParent();
        if (is_declaration) break;
        addSymbol(name);
        break;
      case DW_TAG_namespace:
        if (name.empty()) name = "(anonymous namespace)";
        addParent();
        visitChildren(name);
        break;
      case DW_TAG_typedef:
      case DW_TAG_subrange_type:
        addParent();
        if (is_declaration || name.empty()) break;
        addSymbol(name);
        break;
      case DW_TAG_union_type:
      case DW_TAG_class_type:
      case DW_TAG_interface_type:
      case DW_TAG_structure_type:
      case DW_TAG_enumeration_type:
        addParent();
        if (!is_declaration && !name.empty()) {
          addSymbol(name);
        }
        if (tag == DW_TAG_enumeration_type || !name.empty()) {
          visitChildren(tag == DW_TAG_enumeration_type ? parent_name : name);
        }
        break;
      case DW_TAG_compile_unit:
      case DW_TAG_type_unit:
        visitChildren(parent_name);
        break;
      default:
        break;
    }
  }

  std::pair<std::vector<AddressTableEntry>, SymbolMap>
  collect_addresses_and_symbols(const DwarfState& dwarf) const {
    auto time = ::HPHP::Timer::GetCurrentTimeMicros();

    folly::F14FastMap<uint32_t, uint32_t> unit_indices_cu;
    folly::F14FastMap<uint32_t, uint32_t> unit_indices_tu;

    uint32_t count = 0;
    dwarf.forEachTopLevelUnit(
      [&](Dwarf_Die die) {
        unit_indices_cu.insert({die->context->offset, count});
        count++;
      }, true /* Compilation Unit */
    );
    size_t numCUs = count;
    dwarf.forEachTopLevelUnit(
      [&](Dwarf_Die die) {
        unit_indices_tu[die->context->offset] = count;
        count++;
      }, false /* Type Unit */
    );


    std::vector<std::vector<AddressTableEntry>>
      entryList(numCUs, std::vector<AddressTableEntry>{});
    SymbolMap symbols;

    dwarf.forEachTopLevelUnitParallel(
      [&](Dwarf_Die die) {
        uint32_t index = unit_indices_cu[die->context->offset];
        assertx(index < entryList.size());
        std::vector<AddressTableEntry> entry;
        visit_die_for_address(dwarf, die, entry, index);

        sort(entry.begin(), entry.end(), compareAddressTableEntry);

        std::vector<AddressTableEntry> merged;
        for (auto& e : entry) {
          if (!merged.empty()) {
            auto& prev = merged.back();
            if (e.low <= prev.high) {
              if (e.high <= prev.high) continue;
              assertx(prev.index == e.index);
              prev.high = e.high;
              continue;
            }
          }
          merged.push_back(e);
        }

        entryList[index] = std::move(merged);
        SpecMap spec_names;
        visit_die_for_symbols(dwarf, die, symbols, spec_names, "",
                              0, index);
      }, true /* Compilation Unit */, m_numThreads
    );

    std::vector<AddressTableEntry> entries;
    for (auto& list : entryList) {
      for (auto &e : list) {
        entries.push_back(e);
      }
    }

    time = log_time(time, "collect_addresses_and_symbols: Visit CUs");

    dwarf.forEachTopLevelUnitParallel(
      [&](Dwarf_Die die) {
        uint32_t index = unit_indices_tu[die->context->offset];
        SpecMap spec_names;
        visit_die_for_symbols(dwarf, die, symbols, spec_names, "",
                              0, index);
      }, false /* Type Unit */, m_numThreads
    );

    log_time(time, "collect_addresses_and_symbols: Visit TUs");

    return {std::move(entries), std::move(symbols)};
  }

  struct SymbolAndConstantPool {
    GDBHashtable symbol_pool;
    std::vector<uint32_t> cu_vector_offsets;
    std::vector<std::string> strings;
  };

  SymbolAndConstantPool
  get_symbol_and_constants(const SymbolMap& symbols) const {
    auto time = ::HPHP::Timer::GetCurrentTimeMicros();

    GDBHashtable symbol_hash_table;
    symbol_hash_table.init(symbols.size());

    auto const getHashVal = [](std::string name) {
      uint32_t r = 0;
      for (char& c : name) {
        c = tolower(c);
        r = r * 67 + c - 113;
      }
      return r;
    };

    // The first value is the number of CU indices in the vector
    std::vector<uint32_t> cu_vector_values;
    std::vector<std::string> strings;

    // set name_off to 1 so can use non-zero as the valid test for a
    // hash table entry.
    uint32_t name_off = 1;
    for (auto& entry : symbols) {
      uint32_t cu_vector_offset = cu_vector_values.size() * 4;
      cu_vector_values.push_back(entry.second.size());
      for (auto& elem : entry.second) {
        cu_vector_values.push_back(elem);
      }
      strings.push_back(entry.first);
      symbol_hash_table.add(getHashVal(entry.first),
                            GDBSymbol{name_off, cu_vector_offset});
      name_off += entry.first.length() + 1;
    }

    time = log_time(time, "Get_symbol_and_constants: Populate hash table");

    auto const num_cu_vector_bytes =
      cu_vector_values.size() * sizeof(cu_vector_values[0]);
    for (auto& sym : symbol_hash_table.m_hashtable) {
      if (sym.valid()) {
        sym.name_offset += num_cu_vector_bytes - 1;
      }
    }

    log_time(time, "Get_symbol_and_constants: Update symbol pool");

    std::cout << "Hash Table Size: " << symbol_hash_table.m_size <<
      " Capacity: " << symbol_hash_table.m_capacity << std::endl;
    std::cout << "Strings Size: " << strings.size() << std::endl;
    std::cout << "CU Vector Values Size: " <<
      cu_vector_values.size() << std::endl;

    return {
      std::move(symbol_hash_table),
      std::move(cu_vector_values),
      std::move(strings)
    };
  }

  std::string m_filename;
  int m_numThreads;
};

////////////////////////////////////////////////////////////////////////////////

}

std::unique_ptr<TypeParser>
make_dwarf_type_parser(const std::string& filename, int num_threads) {
  return std::make_unique<TypeParserImpl>(filename, num_threads);
}

std::unique_ptr<Printer> make_dwarf_printer(const std::string& filename) {
  return std::make_unique<PrinterImpl>(filename);
}

std::unique_ptr<GDBIndexer>
make_dwarf_gdb_indexer(const std::string& filename, int num_threads) {
  return std::make_unique<GDBIndexerImpl>(filename, num_threads);
}

////////////////////////////////////////////////////////////////////////////////

}

#endif
