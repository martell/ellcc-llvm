//===-- OrcTargetSupport.h - Code to support specific targets  --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Target specific code for Orc, e.g. callback assembly.
//
// Target classes should be part of the JIT *target* process, not the host
// process (except where you're doing hosted JITing and the two are one and the
// same).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXECUTIONENGINE_ORC_ORCTARGETSUPPORT_H
#define LLVM_EXECUTIONENGINE_ORC_ORCTARGETSUPPORT_H

#include "IndirectionUtils.h"
#include "llvm/Support/Memory.h"

namespace llvm {
namespace orc {

class OrcX86_64 {
public:
  static const char *ResolverBlockName;

  /// @brief Insert module-level inline callback asm into module M for the
  /// symbols managed by JITResolveCallbackHandler J.
  static void insertResolverBlock(Module &M,
                                  JITCompileCallbackManagerBase &JCBM);

  /// @brief Get a label name from the given index.
  typedef std::function<std::string(unsigned)> LabelNameFtor;

  /// @brief Insert the requested number of trampolines into the given module.
  /// @param M Module to insert the call block into.
  /// @param NumCalls Number of calls to create in the call block.
  /// @param StartIndex Optional argument specifying the index suffix to start
  ///                   with.
  /// @return A functor that provides the symbol name for each entry in the call
  ///         block.
  ///
  static LabelNameFtor insertCompileCallbackTrampolines(
                                                    Module &M,
                                                    TargetAddress TrampolineAddr,
                                                    unsigned NumCalls,
                                                    unsigned StartIndex = 0);

  /// @brief Provide information about stub blocks generated by the
  ///        makeIndirectStubsBlock function.
  class IndirectStubsInfo {
    friend class OrcX86_64;
  public:
    const static unsigned StubSize = 8;
    const static unsigned PtrSize = 8;

    IndirectStubsInfo() : NumStubs(0) {}
    IndirectStubsInfo(IndirectStubsInfo&&);
    IndirectStubsInfo& operator=(IndirectStubsInfo&&);
    ~IndirectStubsInfo();

    /// @brief Number of stubs in this block.
    unsigned getNumStubs() const { return NumStubs; }

    /// @brief Get a pointer to the stub at the given index, which must be in
    ///        the range 0 .. getNumStubs() - 1.
    void* getStub(unsigned Idx) const {
      return static_cast<uint64_t*>(StubsBlock.base()) + Idx;
    }

    /// @brief Get a pointer to the implementation-pointer at the given index,
    ///        which must be in the range 0 .. getNumStubs() - 1.
    void** getPtr(unsigned Idx) const {
      return static_cast<void**>(PtrsBlock.base()) + Idx;
    }
  private:
    unsigned NumStubs;
    sys::MemoryBlock StubsBlock;
    sys::MemoryBlock PtrsBlock;
  };

  /// @brief Emit at least MinStubs worth of indirect call stubs, rounded out to
  ///        the nearest page size.
  ///
  ///   E.g. Asking for 4 stubs on x86-64, where stubs are 8-bytes, with 4k
  /// pages will return a block of 512 stubs (4096 / 8 = 512). Asking for 513
  /// will return a block of 1024 (2-pages worth).
  static std::error_code emitIndirectStubsBlock(IndirectStubsInfo &StubsInfo,
                                                unsigned MinStubs,
                                                void *InitialPtrVal);
};

} // End namespace orc.
} // End namespace llvm.

#endif // LLVM_EXECUTIONENGINE_ORC_ORCTARGETSUPPORT_H
