Overview
--------

The retranslate all feature performs many retranslations simultaneously. These
threads need space to translate into before they statically relocate to the
final location in the translation cache. If we allocated this space for every
thread, then we'd run out of low memory. Therefore a single piece of low memory
is allocated to be used as a "virtual address space". This is where the worker
threads *think* they're writing the translations, when in fact the assembler
is writing them to the actual location allocated by the worker thread as
scratch space.


X64 usage of toDestAddress()
----------------------------

Any time the compiler is reading or writing a location, it must use the
actual address, which is returned by DataBlock::toDestAddress().

The following is a list of the places that currently do this for the x64
implementation:

-   ~Label() in the X64 Assembler. It patches up any instructions to
    that label.
-   relocateImpl() in relocation-x64.cpp. We copy the src addresses
    to dest addresses one at a time using DataBlock::bytes(). We use
    toDestAddress() then and when we decode instructions.
-   emitSmashableMovq() uses toDestAddress when calculating where to
    write the embedded literal.
-   toReal() in vasm-x64.cpp looks up the code block and does
    toDestAddress(). toReal() is used by:
    -   retargetJumps()
    -   patch()
-   emit mcprep{} in vasm-x64.cpp.


ARM usage of toDestAddress()
----------------------------

ARM similarly must use toDestAddress() a fair amount. The following TODOs
describe this work for ARM.


### toDestAddress() TODOs

*   Update Vixl assembler to use toDestAddress() when necessary.
    *   Update binding/linking with virtual address support. Label::link_ and
        Label::target_ are virtual addresses. When binding a Label, the actual
        instructions linked to it will be updated with target built from the
        offset between the current link_ and target_.
*   Update relocateImpl() and all of the helpers.
    *   Use toDestAddress() when casting to Instruction.
    *   Fix uses of SetImmPCOffset() and ImmPCOffset(). (see next section)
*   Update all uses of __builtin__clear_cache().
    *   align-arm.cpp
    *   func-guard-arm.cpp
    *   relocation-arm.cpp
    *   smashable-instr-arm.cpp
    x   tc-relocate.cpp
    *   unique-stubs-arm.cpp
    *   vasm-arm.cpp
*   emitSmashable*. Every place we emit a smashable, we need to get
    the actual address to store in the smashableLocations.
*   Any place in vasm-arm.cpp similiar to those that use toReal() in
    vasm-x64.cpp.
    *   patch()
*   emit mcprep{} in vasm-arm.cpp.
*   emit jmp{} and jcc{}.

Additionally, ARM's relocation code does a *lot* of analysis of PC relative
instructions which includes their targets. The trick here is that these targets
point into the weeds. The idea of using the virtual address space is that once
we relocate them into their resting point, the instructions *should* still be PC
relative. Therefore when emitting these PC relative instruction, we use a branch
distance from the virtual address to the target in the actual code cache.
However, that distance applied to to physical address in the thread local code
cache will point into the sticks. To counteract this, two key vixl::Instruction
helper functions are modified to take a `from` argument. If that argument is
non-null, then all offsets are calculated as if they were from that `from`
address and not the actual address.

### ARM PC Relative Instruction Details

-   Instructions are linked to a Label during their emission upon the call to:
    -   Label::UpdateAndGetByteOffsetTo()
    -   Label::UpdateAndGetInstructionOffsetTo()
    -   If the Label is already bound, then the instruction gets the proper
            address,
    -   If the Label is not yet bound, then it is inserted into the head of the
        Label's linked list. The offset returned is the 0 this is the first
        linked instruction. Otherwise it is the offset to the Instruction which
        is the previous head of the list. The Label's link is then set to this
        new Instruction as the new head.
-   Labels are bound through a call to:
    -   Label::bind()
    -   If the Label has been linked, then each Instruction in the chain has
        their offset updated to point to the new bound location.
-   PC Relative Instruction targets are manipulated with calls to:
    -   Instruction::SetImmPCOffsetTarget()
    -   Instruction::ImmPCOffsetTarget()
    -   These are used in:
        -   smashable-instr-arm.cpp: Code is published, so no updates needed.
        -   Label::bind() - Needs updating. (DONE)
        -   relocation-test.cpp: No updates needed.
        -   relocation-arm.cpp: Needs updating.
