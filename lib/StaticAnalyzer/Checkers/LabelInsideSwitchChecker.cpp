//==- CheckSizeofPointer.cpp - Check for sizeof on pointers ------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "ClangSACheckers.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"

using namespace clang;
using namespace ento;

namespace {
class WalkAST : public StmtVisitor<WalkAST> {
  BugReporter &BR;
  AnalysisDeclContext* AC;
  const CheckerBase *Checker;
public:
  WalkAST(BugReporter &br, const CheckerBase *ch,
          AnalysisDeclContext* ac) : BR(br),Checker(ch), AC(ac) {}
  void VisitStmt(Stmt *S) { VisitChildren(S); }
  void VisitChildren(Stmt *S);
  void VisitSwitchStmt(SwitchStmt *SS);
  LabelStmt *getLabelStmt(Stmt *S);
};
}
LabelStmt *WalkAST::getLabelStmt(Stmt *S) {
  if (LabelStmt *LS = dyn_cast<LabelStmt>(S))
    return LS;
  for (Stmt::child_iterator I = S->child_begin(), E = S->child_end(); I!=E; ++I)
    if (Stmt *child = *I) {
      LabelStmt *LS = getLabelStmt(child);
      if (LS != NULL)
        return LS;
    }
  return NULL;
}
void WalkAST::VisitSwitchStmt(SwitchStmt *SS) {
  //SS->dump();
  if (SS->getLocStart().isMacroID())
    return;
  LabelStmt *LS = getLabelStmt(SS->getBody());

  if (LS != NULL) {
    llvm::StringRef LabelName(LS->getName());
    if ((LabelName.front() == 'd' && LabelName.back() == 't') ||
        (LabelName.front() == 'c' && LabelName.back() == 'e')) {
      SourceRange R = LS->getSourceRange();
      PathDiagnosticLocation ELoc =
          PathDiagnosticLocation::createBegin(LS, BR.getSourceManager(), AC);
      BR.EmitBasicReport(AC->getDecl(), Checker,
          "Label inside switch",
          "Logic",
          "Possible misprint: label found inside the switch statement",
          ELoc, R);
    }
  }
}

void WalkAST::VisitChildren(Stmt *S) {
  for (Stmt::child_iterator I = S->child_begin(), E = S->child_end(); I!=E; ++I)
    if (Stmt *child = *I)
      Visit(child);
}

namespace {
class LabelInsideSwitchChecker : public Checker<check::ASTCodeBody> {
public:
  void checkASTCodeBody(const Decl *D, AnalysisManager& mgr,
      BugReporter &BR) const {
    WalkAST walker(BR, this, mgr.getAnalysisDeclContext(D));
    walker.Visit(D->getBody());
  }
};
}

void ento::registerLabelInsideSwitchChecker(CheckerManager &mgr) {
  mgr.registerChecker<LabelInsideSwitchChecker>();
}
