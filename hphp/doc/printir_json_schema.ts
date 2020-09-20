type BlockId = number; // unsigned int
type CounterName = string; // uncertain of format
type DisasmString = string; // uncertain of format
type ExtraString = string; // uncertain of format
type FileName = string; //uncertain of format
type FuncName = string; // uncertain of format
type FuncString = string; // uncertain of format
type GuardConstraintString = string; // uncertain of format
type InstrId = number; // uint32_t
type LineNum = number; // int
type Offset = number; // int
type Opcode = string; // Some sort of enum
type OptIndex = number; // int
type ProfCount = number; // uint64_t
type ProfileString = string; // uncertain of format
type SSATmpId = number; // uint32_t
type TCA = string;
type TransId = number; // int32_t
type TypeString = string; // uncertain of format
type UnitString = string; // uncertain of format

type Unit = {
  blocks: [Block];
  translation: TransContext;
  opcodeStats: OpcodeStats;
  inliningDecisions: [InliningDecision];
};

type Block = {
  label: LabelInfo;
  profCount: ProfCount;
  preds: [BlockId];
  next: LabelInfo | null;
  instrs: [Instr];
  area: Area;
};

type LabelInfo = {
  id: BlockId;
  isCatch: boolean;
  hint: Hint;
};

enum Hint {
  Unused,
  Unlikely,
  Neither,
  Likely,
}

enum Area {
  Main,
  Cold,
  Frozen,
}

type Instr = {
  marker: {raw: FuncString} | null; // still not 100% sure what this does
  phiPseudoInstrs: [PhiPseudoInstr];
  opcodeName: Opcode;
  typeParam: TypeString | null;
  guard: GuardConstraintString | "unused" | null;
  extra: ExtraString | null;
  id: InstrId | null;
  taken: LabelInfo | null;
  tc_ranges: [TC_Range] | null // will be null specifically when asmInfo is null
  dsts: [Dst];
  srcs: {counterName: CounterName} | [Src];
  offset: Offset;
  profileData: [ProfileData];
};

type Src = SSATmp;
type Dst = SSATmp;

type SSATmp = {
  id: SSATmpId;
  type: TypeString;
};

type PhiPseudoInstr = {
  srcs: [{
    src: Src;
    label: LabelInfo;
  }];
  dst: Dst;
};

type TC_Range = {
  area: Area;
  start: TCA;
  end: TCA;
  disasm: DisasmString;
}

type ProfileData = {
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

type TransContext = {
  kind: TransKind;
  id: TransId;
  optIndex: OptIndex;
  srcKey: SrcKey;
  funcName: FuncName;
  sourceFile: FileName;
  startLine: LineNum;
  endLine: LineNum;
}

type SrcKey = {
  func: FuncString;
  unit: UnitString;
  prologue: boolean;
  offset: Offset;
  resumeMode: ResumeMode;
  hasThis: boolean;
}

type ResumeMode = "" | "ra" | "rg";

type OpcodeStats = {[x in Opcode] : number;};

type InliningDecision = {
  wasInlined: boolean;
  offset: Offset;
  caller: FuncName;
  callee: FuncName;
  reason: string;
}
