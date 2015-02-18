//== CERT_02_DCL_00.cpp - CERT checker --------------*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// DCL00-C. Const-qualify immutable objects
//
// Immutable objects should be const-qualified.
//
//===----------------------------------------------------------------------===//

#include "ClangSACheckers.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include <map>

using namespace clang;
using namespace ento;

namespace {
  class DCL_00 : public Checker< check::Bind, check::EndAnalysis > {
    mutable std::unique_ptr<BuiltinBug> BT;
        void reportBug(const char *Msg,
                       ProgramStateRef State,
                       CheckerContext &C) const;
    mutable std::map<const VarDecl*, int> VarWriteCounter;
  public:
    void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;
    void checkBind(SVal Loc, SVal  Val, const Stmt *S,
		   CheckerContext &C) const;
    void checkEndAnalysis(ExplodedGraph &G,
			  BugReporter &BR,
			      ExprEngine &Eng) const;
    };
} // end anonymous namespace

REGISTER_MAP_WITH_PROGRAMSTATE(RegionWriteCounter, const MemRegion*, int)

void DCL_00::checkBind(SVal Loc, SVal  Val, const Stmt *S,
                     CheckerContext &C) const {
  const MemRegion *R = Loc.getAsRegion();
  if (!R)
    return;
  const VarRegion *LVR = dyn_cast_or_null<VarRegion>(R);
  if (!LVR)
    return;
  const VarDecl *VD = LVR->getDecl();
  if (!VD->hasInit())
    return;
  if (VD->getType().isConstQualified())
    return;
  ProgramStateRef state = C.getState();
  const int *currentlValue = state->get<RegionWriteCounter>(R);
  ProgramStateRef newState = nullptr;
  if (!currentlValue) {
    newState = state->set<RegionWriteCounter>(R, 1);
  } else {
    int newValue = *currentlValue;
    newState = state->set<RegionWriteCounter>(R, ++newValue);
  }
  C.addTransition(newState);
}

void DCL_00::checkEndAnalysis(ExplodedGraph &G,
			      BugReporter &BR,
			      ExprEngine &Eng) const {

  for (auto I = G.nodes_begin(), E = G.nodes_end(); I != E; I++) {
    ProgramStateRef state = I->getState();
    RegionWriteCounterTy RWC = state->get<RegionWriteCounter>();
    for (auto II = RWC.begin(), EE = RWC.end(); II != EE; II++) {
      const MemRegion *R = II->first;
      const VarRegion *LVR = dyn_cast_or_null<VarRegion>(R);
      if (!LVR)
	continue;
      int value = II->second;
      const VarDecl *VD = LVR->getDecl();
      if (VarWriteCounter[VD] < value)
	VarWriteCounter[VD] = value;
    }
  }

  for (auto I = VarWriteCounter.begin(), End = VarWriteCounter.end(); I != End;
       I++) {
    if (I->second == 1) {
      PathDiagnosticLocation PD =
	PathDiagnosticLocation::createBegin(I->first, BR.getSourceManager());

      BR.EmitBasicReport(I->first, this,
                       "02. Declarations and Initialization (DCL)",
                       categories::LogicError,
			 "DCL00-C. Immutable objects should be const-qualified.",
			 PD, I->first->getSourceRange());
    }
  }

}



void ento::registerDCL_00(CheckerManager &mgr) {
    mgr.registerChecker<DCL_00>();
}
