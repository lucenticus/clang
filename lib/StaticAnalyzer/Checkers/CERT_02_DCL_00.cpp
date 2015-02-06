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
  class DCL_00 : public Checker< check::Location, check::EndAnalysis > {
    mutable std::unique_ptr<BuiltinBug> BT;
        void reportBug(const char *Msg,
                       ProgramStateRef State,
                       CheckerContext &C) const;
    mutable std::map<const VarDecl*, int> VarWriteCounter;
    public:
      void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;
      void checkLocation(SVal Loc, bool IsLoad, const Stmt *S,
			 CheckerContext &) const;
      void checkEndAnalysis(ExplodedGraph &G,
			      BugReporter &BR,
			      ExprEngine &Eng) const;
    };
} // end anonymous namespace

REGISTER_MAP_WITH_PROGRAMSTATE(RegionWriteCounter, const MemRegion*, int)

void DCL_00::reportBug(const char *Msg,
                               ProgramStateRef State,
                               CheckerContext &C) const {

}

void DCL_00::checkLocation(SVal Loc, bool IsLoad, const Stmt *S,
                     CheckerContext &C) const {
  const MemRegion *R = Loc.getAsRegion();
  if (!R)
    return;
  if (!IsLoad)
    return;

  const VarRegion *LVR = dyn_cast_or_null<VarRegion>(Loc.getAsRegion());
  const VarDecl *VD = LVR->getDecl();
  VarWriteCounter[VD]++;
}

void DCL_00::checkEndAnalysis(ExplodedGraph &G,
			      BugReporter &BR,
			      ExprEngine &Eng) const {
  for (auto I = VarWriteCounter.begin(), End = VarWriteCounter.end(); I != End;
       I++) {
    if (I->second == 1) {
      const Expr *E = I->first->getAnyInitializer();
      const AnalysisManager AM = Eng.getAnalysisManager();
      PathDiagnosticLocation PD =
	PathDiagnosticLocation::createBegin(E, AM.getSourceManager(),
					    AM.getAnalysisDeclContext(I->first));
      BR.EmitBasicReport(AM.getAnalysisDeclContext(I->first)->getDecl(), this,
                       "02. Declarations and Initialization (DCL)",
                       categories::LogicError,
			 "DCL00-C. Immutable objects should be const-qualified.",
			 PD, E->getSourceRange());
    }
  }

}



void ento::registerDCL_00(CheckerManager &mgr) {
    mgr.registerChecker<DCL_00>();
}
