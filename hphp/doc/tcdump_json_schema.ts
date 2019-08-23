// Thin types
type ArchNameStr = string;   // uncertain of format
type BCInstrStr = string;    // uncertain of format
type BinaryStr = string;     // uncertain of format, may be arch-dependent
type BlockId = number;       // unsigned int
type CallDestStr = string;   // uncertain of format
type CodeStr = string;       // uncertain of format
type CodeLen = number;       // uint32_t
type ConfigFileStr = string; // maybe empty, should I make this null if empty?
type CounterName = string;   // uncertain of format
type DisasmString = string;  // uncertain of format
type EventCount = number;    // uint64_t
type ExtraString = string;   // uncertain of format
type FileName = string;      // uncertain of format
type FuncId = number;        // uint32_t
type FuncName = string;      // uncertain of format
type FuncString = string;    // uncertain of format
type GuardString = string;   // uncertain of format
type InstrId = number;       // uint32_t
type InstrLen = number       // uint32_t
type LineNum = number;       // int
type Offset = number;        // int32_t
type Opcode = string;        // Some sort of enum
type OptIndex = number;      // int
type ProfCount = number;     // uint64_t
type ProfileString = string; // uncertain of format
type RepoSchemaStr = string; // uncertain of format
type SHA1 = string;          // SHA1.toString()
type SSATmpId = number;      // uint32_t
type TCA = string;           // unsigned char*, casted to void* for sformat
type TransId = number;       // int32_t
type TypeString = string;    // uncertain of format
type UnitFuncStr = string;   // maybe fix? see TODO in tc-print.cpp

type TCDump = {
  configFile: ConfigFileStr;
  repoSchema: RepoSchemaStr;
  translations: [Translation | null];
}

type Translation = {
  transRec: TransRec;
  blocks: [Block];
  archName: ArchNameStr;
  perfEvents: EventCounts;
  regions: {
    main: TCARegionInfo | null;
    cold: TCARegionInfo | null;
    frozen: TCARegionInfo | null;
  };
  transId: TransId;
  ir_annotation: PrintIR_Unit | string;
}

type TransRec = {
  id: TransId;
  src: TransRecSrc;
  kind: TransKind;
  hasLoop: boolean;
  aStart: TCA;
  aLen: CodeLen;
  coldStart: TCA;
  coldLen: CodeLen;
  frozenStart: TCA;
  frozenLen: CodeLen;
}

type TransRecSrc = {
  sha1: SHA1;
  funcId: FuncId;
  funcName: FuncName;
  resumeMode: ResumeMode;
  hasThis: boolean;
  prologue: boolean;
  bcStartOffset: Offset;
  guards: [GuardString];
}

enum ResumeMode {
  None,
  Async,
  GenIter,
}

enum TransKind {
  TransAnchor,
  TransInterp,
  TransLive,
  TransProfile,
  TransOptimize,
  TransLivePrologue,
  TransProfPrologue,
  TransOptPrologue,
  TransInvalid,
}

type Block = {
  sha1: SHA1;
  start: Offset;
  end: Offset;
  unit: UnitFuncStr | null;
}

type EventType =
"cycles" |
"branch-misses" |
"L1-icache-misses" |
"L1-dcache-misses" |
"cache-misses" |
"LLC-store-misses" |
"iTLB-misses" |
"dTLB-misses" |
string; // Technically there can be user-defined events too

type EventCounts = {[event in EventType]: EventCount;}

type TCARegionInfo = {
  tcRegion: TCRegion;
  ranges: [TCARangeInfo];
}

enum TCRegion {
  hot,
  main,
  profile,
  cold,
  frozen
}

type TCARangeInfo = {
  start: TCA;
  end: TCA;
  bc: Offset | null;
  sha1: SHA1 | null;
  instrStr: BCInstrStr | null;
  lineNum: LineNum | null;
  disasm: [TCADisasmInfo];
  ir_annotation?: {
    area: Area;
    start: TCA;
    end: TCA;
    instrId: InstrId;
    blockId: BlockId;
  };
}

type TCADisasmInfo = {
  binary: BinaryStr;
  callDest: CallDestStr;
  code: CodeStr;
  perfEvents: EventCounts;
  ip: TCA;
  instrLen: InstrLen;
}

enum Area {
  Main,
  Cold,
  Frozen
}

type PrintIR_Unit = {
  transContext: PrintIR_TransContext;
  blocks: {[x in string]: PrintIR_Block;};
  // This is actually a map from BlockId to Block, but with
  // the BlockIds interpreted as strings for JSON object compatibility
  inliningDecision: [PrintIR_InliningDecision];
}

type PrintIR_TransContext = {
  kind: TransKind;
  id: TransId;
  optIndex: OptIndex;
  srcKey: PrintIR_SrcKey;
  funcName: FuncName;
  sourceFile: FileName;
  startLine: LineNum;
  endLine: LineNum;
}

type PrintIR_SrcKey = {
  funcStr: FuncString;
  unitStr: UnitFuncString;
  prologue: boolean;
  offset: Offset;
  resumeMode: ResumeMode;
  hasThis: boolean;
}

type ResumeMode = "" | "ra" | "rg";

type PrintIR_Block = {
  id: BlockId;
  isCatch: boolean;
  hint: Hint;
  profCount: ProfCount;
  next: BlockId | null;
  instrs: {[x in string]: PrintIR_Instr;};
  // This is actually a map from InstrId to Instr, but with
  // the InstrIds interpreted as strings for JSON object compatibility
}

enum Hint {
  Unused,
  Unlikely,
  Neither,
  Likely,
}

type PrintIR_Instr = {
  rawMarker: FuncString | null;
  phiPseudoInstrs: [PrintIR_PhiPseudoInstrs];
  opcode: Opcode;
  typeParam: TypeString | null;
  guard: GuardString | null;
  extra: ExtraString | null;
  id: InstrId;
  taken: BlockId | null;
  tcRanges: [PrintIR_TCRange];
  dsts: [PrintIR_SSATmp];
  offset: Offset;
  profileData: PrintIR_Profile;
  srcs: [PrintIR_SSATmp] | null;   // exactly one of srcs and counterName should
  counterName: CounterName | null; // be defined
}

type PrintIR_PhiPseudoInstrs = {
  srcs: [[PrintIR_SSATmp, BlockId]];
  dst: PrintIR_SSATmp;
}

type PrintIR_SSATmp = {
  id: SSATmpId;
  type: TypeString;
}

type PrintIR_TCRange = {
  area: Area;
  start: TCA;
  end: TCA;
  disasm: string;
}

type PrintIR_Profile = {
  offset: Offset;
  name: ProfileString;
  data: {profileType: ProfileType};
  // the rest of the keys in "data" will depend on the value of "profileType"
}

enum ProfileType {
  ArrayAccessProfile,
  ArrayKindProfile,
  CallTargetProfile,
  ClsCnsProfile,
  DecRefProfile,
  IncRefProfile,
  MethProfile,
  ReleaseVVProfile,
  SwitchProfile,
  TypeProfile,
}

type PrintIR_InliningDecision = {
  wasInlined: boolean;
  offset: Offset;
  callerName: FuncName | null;
  calleeName: FuncName | null;
  reason: string;
}
