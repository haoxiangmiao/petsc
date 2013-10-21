#include <../src/snes/impls/gs/gsimpl.h>

#undef __FUNCT__
#define __FUNCT__ "GSDestroy_Private"
PetscErrorCode GSDestroy_Private(ISColoring coloring)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = ISColoringDestroy(&coloring);CHKERRQ(ierr);
  PetscFunctionReturn(0);

}

#undef __FUNCT__
#define __FUNCT__ "SNESComputeGSDefaultSecant"
PETSC_EXTERN PetscErrorCode SNESComputeGSDefaultSecant(SNES snes,Vec X,Vec B,void *ctx)
{
  PetscErrorCode ierr;
  SNES_GS *gs = (SNES_GS*)snes->data;
  PetscInt       i,j,k,ncolors;
  DM             dm;
  PetscBool      flg;
  ISColoring     coloring;
  MatColoring    mc;
  Vec            W,G,F;
  PetscScalar    h=gs->h;
  IS             *coloris;
  PetscScalar    f,g,x,w,d;
  PetscReal      dxt,xt,ft,ft1;
  const PetscInt *idx;
  PetscInt       size;
  PetscReal      atol,rtol,stol;
  PetscInt       its;
  PetscErrorCode (*func)(SNES,Vec,Vec,void*);
  void           *fctx;
  PetscContainer colorcontainer;
  PetscBool      mat = gs->secant_mat,equal;

  PetscFunctionBegin;
  if (snes->nwork < 3) {
    ierr = SNESSetWorkVecs(snes,3);CHKERRQ(ierr);
  }
  W = snes->work[0];
  G = snes->work[1];
  F = snes->work[2];
  ierr = SNESGSGetTolerances(snes,&atol,&rtol,&stol,&its);CHKERRQ(ierr);
  ierr = SNESGetDM(snes,&dm);CHKERRQ(ierr);
  ierr = SNESGetFunction(snes,NULL,&func,&fctx);CHKERRQ(ierr);
  ierr = PetscObjectQuery((PetscObject)snes,"SNESGSColoring",(PetscObject*)&colorcontainer);CHKERRQ(ierr);
  if (!colorcontainer) {
    /* create the coloring */
    ierr = DMHasColoring(dm,&flg);CHKERRQ(ierr);
    if (flg && !mat) {
      ierr = DMCreateColoring(dm,IS_COLORING_GLOBAL,&coloring);CHKERRQ(ierr);
    } else {
      if (!snes->jacobian) {ierr = SNESSetUpMatrices(snes);CHKERRQ(ierr);}
      ierr = MatColoringCreate(snes->jacobian,&mc);CHKERRQ(ierr);
      ierr = MatColoringSetDistance(mc,1);CHKERRQ(ierr);
      ierr = MatColoringSetFromOptions(mc);CHKERRQ(ierr);
      ierr = MatColoringApply(mc,&coloring);CHKERRQ(ierr);
      ierr = MatColoringDestroy(&mc);CHKERRQ(ierr);
    }
    ierr = PetscContainerCreate(PetscObjectComm((PetscObject)snes),&colorcontainer);CHKERRQ(ierr);
    ierr = PetscContainerSetPointer(colorcontainer,(void *)coloring);CHKERRQ(ierr);
    ierr = PetscContainerSetUserDestroy(colorcontainer,(PetscErrorCode (*)(void *))GSDestroy_Private);CHKERRQ(ierr);
    ierr = PetscObjectCompose((PetscObject)snes,"SNESGSColoring",(PetscObject)colorcontainer);CHKERRQ(ierr);
    ierr = PetscContainerDestroy(&colorcontainer);CHKERRQ(ierr);
  } else {
    ierr = PetscContainerGetPointer(colorcontainer,(void **)&coloring);CHKERRQ(ierr);
  }
  ierr = ISColoringGetIS(coloring,&ncolors,&coloris);CHKERRQ(ierr);
  ierr = VecEqual(X,snes->vec_sol,&equal);CHKERRQ(ierr);
  if (equal) {
    /* assume that the function is already computed */
    ierr = VecCopy(snes->vec_func,F);CHKERRQ(ierr);
  } else {
    ierr = PetscLogEventBegin(SNES_GSFuncEval,snes,X,B,0);CHKERRQ(ierr);
    ierr = (*func)(snes,X,F,fctx);CHKERRQ(ierr);
    ierr = PetscLogEventEnd(SNES_GSFuncEval,snes,X,B,0);CHKERRQ(ierr);
    if (B) {ierr = VecAXPY(F,-1.0,B);CHKERRQ(ierr);}
  }
  for (i=0;i<ncolors;i++) {
    ierr = ISGetIndices(coloris[i],&idx);CHKERRQ(ierr);
    ierr = ISGetLocalSize(coloris[i],&size);CHKERRQ(ierr);
    ierr = VecCopy(X,W);CHKERRQ(ierr);
    for (j=0;j<size;j++) {
      ierr = VecSetValue(W,idx[j],h,ADD_VALUES);CHKERRQ(ierr);
    }
    ierr = PetscLogEventBegin(SNES_GSFuncEval,snes,X,B,0);CHKERRQ(ierr);
    ierr = (*func)(snes,W,G,fctx);CHKERRQ(ierr);
    ierr = PetscLogEventEnd(SNES_GSFuncEval,snes,X,B,0);CHKERRQ(ierr);
    if (B) {ierr = VecAXPY(G,-1.0,B);CHKERRQ(ierr);}
    for (k=0;k<its;k++) {
      dxt = 0.;
      xt = 0.;
      for (j=0;j<size;j++) {
        ierr = VecGetValues(F,1,&idx[j],&f);CHKERRQ(ierr);
        ierr = VecGetValues(X,1,&idx[j],&x);CHKERRQ(ierr);
        ierr = VecGetValues(G,1,&idx[j],&g);CHKERRQ(ierr);
        ierr = VecGetValues(W,1,&idx[j],&w);CHKERRQ(ierr);
        if (PetscAbsScalar(g-f) > atol) {
          d = (x*g-w*f) / PetscRealPart(g-f);
        } else {
          d = x;
        }
        dxt += (d-x)*(d-x);
        xt += x*x;
        ft += f*f;
        ierr = VecSetValue(X,idx[j],d,INSERT_VALUES);CHKERRQ(ierr);
      }
      if (k == 0) ft1 = PetscSqrtScalar(ft);
      if (stol*PetscSqrtReal(xt) > PetscSqrtReal(dxt)) break;
      if (PetscSqrtReal(ft) < atol) break;
      if (rtol*ft1 > PetscSqrtReal(ft)) break;
      if (i < ncolors-1 || k < its-1) {
        ierr = PetscLogEventBegin(SNES_GSFuncEval,snes,X,B,0);CHKERRQ(ierr);
        ierr = (*func)(snes,X,F,fctx);CHKERRQ(ierr);
        ierr = PetscLogEventEnd(SNES_GSFuncEval,snes,X,B,0);CHKERRQ(ierr);
        if (B) {ierr = VecAXPY(F,-1.0,B);CHKERRQ(ierr);}
      }
      if (k<its-1) {
        ierr = VecSwap(X,W);CHKERRQ(ierr);
        ierr = VecSwap(F,G);CHKERRQ(ierr);
      }
    }
  }
  ierr = ISColoringRestoreIS(coloring,&coloris);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
