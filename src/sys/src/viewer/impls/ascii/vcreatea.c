#ifdef PETSC_RCS_HEADER
static char vcid[] = "$Id: filev.c,v 1.81 1999/01/12 23:17:24 bsmith Exp bsmith $";
#endif

#include "src/viewer/viewerimpl.h"  /*I     "petsc.h"   I*/
#include "pinclude/petscfix.h"
#include <stdarg.h>

typedef struct {
  FILE          *fd;
  int           tab;   /* how many times text is tabbed in from left */
} Viewer_ASCII;

Viewer VIEWER_STDOUT_SELF, VIEWER_STDERR_SELF, VIEWER_STDOUT_WORLD, VIEWER_STDERR_WORLD;

/*
      This is called by PETScInitialize() to create the 
   default PETSc viewers.
*/
#undef __FUNC__  
#define __FUNC__ "ViewerInitializeASCII_Private"
int ViewerInitializeASCII_Private(void)
{
  int ierr;
  PetscFunctionBegin;
  ierr = ViewerASCIIOpen(PETSC_COMM_SELF,"stderr",&VIEWER_STDERR_SELF);CHKERRQ(ierr);
  ierr = ViewerASCIIOpen(PETSC_COMM_SELF,"stdout",&VIEWER_STDOUT_SELF);CHKERRQ(ierr);
  ierr = ViewerASCIIOpen(PETSC_COMM_WORLD,"stdout",&VIEWER_STDOUT_WORLD);CHKERRQ(ierr);
  ierr = ViewerASCIIOpen(PETSC_COMM_WORLD,"stderr",&VIEWER_STDERR_WORLD);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/*
      This is called in PetscFinalize() to destroy all
   traces of the default viewers.
*/
#undef __FUNC__  
#define __FUNC__ "ViewerDestroyASCII_Private"
int ViewerDestroyASCII_Private(void)
{
  int ierr;

  PetscFunctionBegin;
  ierr = ViewerDestroy(VIEWER_STDERR_SELF);CHKERRQ(ierr);
  ierr = ViewerDestroy(VIEWER_STDOUT_SELF);CHKERRQ(ierr);
  ierr = ViewerDestroy(VIEWER_STDOUT_WORLD);CHKERRQ(ierr);
  ierr = ViewerDestroy(VIEWER_STDERR_WORLD);CHKERRQ(ierr);
  ierr = VIEWER_STDOUT_Destroy(PETSC_COMM_SELF);CHKERRQ(ierr);
  ierr = VIEWER_STDERR_Destroy(PETSC_COMM_SELF);CHKERRQ(ierr);
  ierr = VIEWER_STDOUT_Destroy(PETSC_COMM_WORLD);CHKERRQ(ierr);
  ierr = VIEWER_STDERR_Destroy(PETSC_COMM_WORLD);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/* ---------------------------------------------------------------------*/
/*
    The variable Petsc_Viewer_Stdout_keyval is used to indicate an MPI attribute that
  is attached to a communicator, in this case the attribute is a Viewer.
*/
static int Petsc_Viewer_Stdout_keyval = MPI_KEYVAL_INVALID;

#undef __FUNC__  
#define __FUNC__ "VIEWER_STDOUT_" 
/*@C
   VIEWER_STDOUT_ - Creates a window viewer shared by all processors 
                    in a communicator.

   Collective on MPI_Comm

   Input Parameter:
.  comm - the MPI communicator to share the window viewer

   Notes: 
   Unlike almost all other PETSc routines, this does not return 
   an error code. Usually used in the form
$      XXXView(XXX object,VIEWER_STDOUT_(comm));

.seealso: VIEWER_DRAWX_(), ViewerASCIIOpen()

@*/
Viewer VIEWER_STDOUT_(MPI_Comm comm)
{
  int    ierr,flag;
  Viewer viewer;

  PetscFunctionBegin;
  if (Petsc_Viewer_Stdout_keyval == MPI_KEYVAL_INVALID) {
    ierr = MPI_Keyval_create(MPI_NULL_COPY_FN,MPI_NULL_DELETE_FN,&Petsc_Viewer_Stdout_keyval,0);
    if (ierr) {PetscError(__LINE__,"VIEWER_STDOUT_",__FILE__,__SDIR__,1,1,0); viewer = 0;}
  }
  ierr = MPI_Attr_get( comm, Petsc_Viewer_Stdout_keyval, (void **)&viewer, &flag );
  if (ierr) {PetscError(__LINE__,"VIEWER_STDOUT_",__FILE__,__SDIR__,1,1,0); viewer = 0;}
  if (!flag) { /* viewer not yet created */
    ierr = ViewerASCIIOpen(comm,"stdout",&viewer);
    if (ierr) {PetscError(__LINE__,"VIEWER_STDOUT_",__FILE__,__SDIR__,1,1,0); viewer = 0;}
    ierr = MPI_Attr_put( comm, Petsc_Viewer_Stdout_keyval, (void *) viewer );
    if (ierr) {PetscError(__LINE__,"VIEWER_STDOUT_",__FILE__,__SDIR__,1,1,0); viewer = 0;}
  } 
  PetscFunctionReturn(viewer);
}

/*
       If there is a Viewer associated with this communicator it is destroyed.
*/
int VIEWER_STDOUT_Destroy(MPI_Comm comm)
{
  int    ierr,flag;
  Viewer viewer;

  PetscFunctionBegin;
  if (Petsc_Viewer_Stdout_keyval == MPI_KEYVAL_INVALID) {
    PetscFunctionReturn(0);
  }
  ierr = MPI_Attr_get( comm, Petsc_Viewer_Stdout_keyval, (void **)&viewer, &flag ); CHKERRQ(ierr);
  if (flag) { 
    ierr = ViewerDestroy(viewer); CHKERRQ(ierr);
    ierr = MPI_Attr_delete(comm,Petsc_Viewer_Stdout_keyval); CHKERRQ(ierr);
  } 
  PetscFunctionReturn(0);
}

/* ---------------------------------------------------------------------*/
/*
    The variable Petsc_Viewer_Stderr_keyval is used to indicate an MPI attribute that
  is attached to a communicator, in this case the attribute is a Viewer.
*/
static int Petsc_Viewer_Stderr_keyval = MPI_KEYVAL_INVALID;

#undef __FUNC__  
#define __FUNC__ "VIEWER_STDERR_" 
/*@C
   VIEWER_STDERR_ - Creates a window viewer shared by all processors 
                    in a communicator.

   Collective on MPI_Comm

   Input Parameter:
.  comm - the MPI communicator to share the window viewer

   Note: 
   Unlike almost all other PETSc routines, this does not return 
   an error code. Usually used in the form
$      XXXView(XXX object,VIEWER_STDERR_(comm));

.seealso: VIEWER_DRAWX_, ViewerASCIIOpen(), 
@*/
Viewer VIEWER_STDERR_(MPI_Comm comm)
{
  int    ierr,flag;
  Viewer viewer;

  PetscFunctionBegin;
  if (Petsc_Viewer_Stderr_keyval == MPI_KEYVAL_INVALID) {
    ierr = MPI_Keyval_create(MPI_NULL_COPY_FN,MPI_NULL_DELETE_FN,&Petsc_Viewer_Stderr_keyval,0);
    if (ierr) {PetscError(__LINE__,"VIEWER_STDERR_",__FILE__,__SDIR__,1,1,0); viewer = 0;}
  }
  ierr = MPI_Attr_get( comm, Petsc_Viewer_Stderr_keyval, (void **)&viewer, &flag );
  if (ierr) {PetscError(__LINE__,"VIEWER_STDERR_",__FILE__,__SDIR__,1,1,0); viewer = 0;}
  if (!flag) { /* viewer not yet created */
    ierr = ViewerASCIIOpen(comm,"stderr",&viewer);
    if (ierr) {PetscError(__LINE__,"VIEWER_STDERR_",__FILE__,__SDIR__,1,1,0); viewer = 0;}
    ierr = MPI_Attr_put( comm, Petsc_Viewer_Stderr_keyval, (void *) viewer );
    if (ierr) {PetscError(__LINE__,"VIEWER_STDERR_",__FILE__,__SDIR__,1,1,0); viewer = 0;}
  } 
  PetscFunctionReturn(viewer);
}

/*
       If there is a Viewer associated with this communicator it is destroyed.
*/
int VIEWER_STDERR_Destroy(MPI_Comm comm)
{
  int    ierr,flag;
  Viewer viewer;

  PetscFunctionBegin;
  if (Petsc_Viewer_Stderr_keyval == MPI_KEYVAL_INVALID) {
    PetscFunctionReturn(0);
  }
  ierr = MPI_Attr_get( comm, Petsc_Viewer_Stderr_keyval, (void **)&viewer, &flag );CHKERRQ(ierr);
  if (flag) { 
    ierr = ViewerDestroy(viewer); CHKERRQ(ierr);
    ierr = MPI_Attr_delete(comm,Petsc_Viewer_Stderr_keyval);CHKERRQ(ierr);
  } 
  PetscFunctionReturn(0);
}

/* ----------------------------------------------------------------------*/
#undef __FUNC__  
#define __FUNC__ "ViewerDestroy_ASCII"
int ViewerDestroy_ASCII(Viewer v)
{
  int          rank = 0;
  Viewer_ASCII *vascii = (Viewer_ASCII *)v->data;

  PetscFunctionBegin;
  if (!rank && vascii->fd != stderr && vascii->fd != stdout) fclose(vascii->fd);
  PetscFree(vascii);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ViewerFlush_ASCII"
int ViewerFlush_ASCII(Viewer v)
{
  int          rank;
  Viewer_ASCII *vascii = (Viewer_ASCII *)v->data;

  PetscFunctionBegin;
  MPI_Comm_rank(v->comm,&rank);
  if (rank) PetscFunctionReturn(0);
  fflush(vascii->fd);
  PetscFunctionReturn(0);  
}

#undef __FUNC__  
#define __FUNC__ "ViewerASCIIGetPointer"
/*@C
    ViewerASCIIGetPointer - Extracts the file pointer from an ASCII viewer.

    Not Collective

+   viewer - viewer context, obtained from ViewerASCIIOpen()
-   fd - file pointer

    Fortran Note:
    This routine is not supported in Fortran.

.keywords: Viewer, file, get, pointer

.seealso: ViewerASCIIOpen()
@*/
int ViewerASCIIGetPointer(Viewer viewer, FILE **fd)
{
  Viewer_ASCII *vascii = (Viewer_ASCII *)viewer->data;

  PetscFunctionBegin;
  *fd = vascii->fd;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ViewerASCIIGetOutputname"
int ViewerGetOutputname(Viewer viewer, char **name)
{
  PetscFunctionBegin;
  *name = viewer->outputname;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ViewerGetFormat"
int ViewerGetFormat(Viewer viewer,int *format)
{
  PetscFunctionBegin;
  *format =  viewer->format;
  PetscFunctionReturn(0);
}

/*
   If petsc_history is on, then all Petsc*Printf() results are saved
   if the appropriate (usually .petschistory) file.
*/
extern FILE *petsc_history;

#undef __FUNC__  
#define __FUNC__ "ViewerASCIIPushTab" 
/*@C
    ViewerASCIIPushTab - Adds one more tab to the amount that ViewerASCIIPrintf()
     lines are tabbed.

    Not Collective, but only first processor in set has any effect

    Input Parameters:
.    viewer - optained with ViewerASCIIOpen()

    Fortran Note:
    This routine is not supported in Fortran.

.keywords: parallel, fprintf

.seealso: PetscPrintf(), PetscSynchronizedPrintf(), ViewerASCIIPrintf(),
          ViewerASCIIPopTab()
@*/
int ViewerASCIIPushTab(Viewer viewer)
{
  Viewer_ASCII *ascii = (Viewer_ASCII*) viewer->data;

  PetscFunctionBegin;
  if (!PetscTypeCompare(viewer->type_name,ASCII_VIEWER)) PetscFunctionReturn(0);
  ascii->tab++;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ViewerASCIIPopTab" 
/*@C
    ViewerASCIIPopTab - Removes one tab from the amount that ViewerASCIIPrintf()
     lines are tabbed.

    Not Collective, but only first processor in set has any effect

    Input Parameters:
.    viewer - optained with ViewerASCIIOpen()

    Fortran Note:
    This routine is not supported in Fortran.

.keywords: parallel, fprintf

.seealso: PetscPrintf(), PetscSynchronizedPrintf(), ViewerASCIIPrintf(),
          ViewerASCIIPushTab()
@*/
int ViewerASCIIPopTab(Viewer viewer)
{
  Viewer_ASCII *ascii = (Viewer_ASCII*) viewer->data;

  PetscFunctionBegin;
  if (!PetscTypeCompare(viewer->type_name,ASCII_VIEWER)) PetscFunctionReturn(0);
  if (ascii->tab <= 0) SETERRQ(1,1,"More tabs popped than pushed");
  ascii->tab--;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ViewerASCIIPrintf" 
/*@C
    ViewerASCIIPrintf - Prints to a file, only from the first
    processor in the viewer

    Not Collective, but only first processor in set has any effect

    Input Parameters:
+    viewer - optained with ViewerASCIIOpen()
-    format - the usual printf() format string 

    Fortran Note:
    This routine is not supported in Fortran.

.keywords: parallel, fprintf

.seealso: PetscPrintf(), PetscSynchronizedPrintf(), ViewerASCIIOpen(),
          ViewerASCIIPushTab(), ViewerASCIIPopTab()
@*/
int ViewerASCIIPrintf(Viewer viewer,const char format[],...)
{
  Viewer_ASCII *ascii = (Viewer_ASCII*) viewer->data;
  int          rank, tab;
  FILE         *fd = ascii->fd;

  PetscFunctionBegin;
  MPI_Comm_rank(viewer->comm,&rank);
  if (!rank) {
    va_list Argp;

    tab = ascii->tab;
    while (tab--) fprintf(fd,"  ");

    va_start( Argp, format );
#if defined(HAVE_VPRINTF_CHAR)
    vfprintf(fd,format,(char*)Argp);
#else
    vfprintf(fd,format,Argp);
#endif
    fflush(fd);
    if (petsc_history) {
      tab = ascii->tab;
      while (tab--) fprintf(fd,"  ");
#if defined(HAVE_VPRINTF_CHAR)
      vfprintf(petsc_history,format,(char *)Argp);
#else
      vfprintf(petsc_history,format,Argp);
#endif
      fflush(petsc_history);
    }
    va_end( Argp );
  }
  PetscFunctionReturn(0);
}

/*@
     ViewerSetFilename - Sets the name of the file the viewer uses.

    Collective on Viewer

  Input Parameters:
+  viewer - the viewer; either ASCII or binary
-  name - the name of the file it should use

.seealso: ViewerCreate(), ViewerSetType(), ViewerASCIIOpen(), ViewerBinaryOpen()

@*/
#undef __FUNC__  
#define __FUNC__ "ViewerSetFilename"
int ViewerSetFilename(Viewer viewer,const char name[])
{
  int ierr, (*f)(Viewer viewer,const char name[]);

  PetscFunctionBegin;
  PetscValidHeaderSpecific(viewer,VIEWER_COOKIE);
  ierr = PetscObjectQueryFunction((PetscObject)viewer,"ViewerSetFilename_C",(void **)&f);CHKERRQ(ierr);
  if (f) {
    ierr = (*f)(viewer,name);CHKERRQ(ierr);
  }

  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ViewerSetFilename_ASCII"
int ViewerSetFilename_ASCII(Viewer viewer,const char name[])
{
  int          ierr;
  char         fname[256];
  Viewer_ASCII *vascii = (Viewer_ASCII *) viewer->data;

  PetscFunctionBegin;
  if (!name) PetscFunctionReturn(0);

  if (!PetscStrcmp(name,"stderr"))      vascii->fd = stderr;
  else if (!PetscStrcmp(name,"stdout")) vascii->fd = stdout;
  else {
    ierr         = PetscFixFilename(name,fname);CHKERRQ(ierr);
    vascii->fd   = fopen(fname,"w"); 
    if (!vascii->fd) SETERRQ(PETSC_ERR_FILE_OPEN,0,"Cannot open viewer file");
  }
#if defined(USE_PETSC_LOG)
  PLogObjectState((PetscObject)viewer,"File: %s",name);
#endif

  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ViewerASCIIOpen"
/*@C
   ViewerASCIIOpen - Opens an ASCII file as a viewer.

   Collective on MPI_Comm

   Input Parameters:
+  comm - the communicator
-  name - the file name

   Output Parameter:
.  lab - the viewer to use with the specified file

   Notes:
   This viewer can be destroyed with ViewerDestroy().

   If a multiprocessor communicator is used (such as PETSC_COMM_WORLD), 
   then only the first processor in the group opens the file.  All other 
   processors send their data to the first processor to print. 

   Each processor can instead write its own independent output by
   specifying the communicator PETSC_COMM_SELF.

   As shown below, ViewerASCIIOpen() is useful in conjunction with 
   MatView() and VecView()
.vb
     ViewerASCIIOpen(PETSC_COMM_WORLD,"mat.output",&viewer);
     MatView(matrix,viewer);
.ve

.keywords: Viewer, file, open

.seealso: MatView(), VecView(), ViewerDestroy(), ViewerBinaryOpen(),
          ViewerASCIIGetPointer()
@*/
int ViewerASCIIOpen(MPI_Comm comm,const char name[],Viewer *lab)
{
  int ierr;

  PetscFunctionBegin;
  ierr = ViewerCreate(comm,lab);CHKERRQ(ierr);
  ierr = ViewerSetType(*lab,ASCII_VIEWER);CHKERRQ(ierr);
  if (name) {
    ierr = ViewerSetFilename(*lab,name);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}


EXTERN_C_BEGIN
#undef __FUNC__  
#define __FUNC__ "ViewerCreate_ASCII"
int ViewerCreate_ASCII(Viewer v)
{
  Viewer_ASCII *vascii;
  int          ierr;

  PetscFunctionBegin;
  vascii  = PetscNew(Viewer_ASCII);CHKPTRQ(vascii);
  v->data = (void *) vascii;

  PLogObjectCreate(v);
  v->ops->destroy     = ViewerDestroy_ASCII;
  v->ops->flush       = ViewerFlush_ASCII;

  /* defaults to stdout unless set with ViewerSetFilename() */
  vascii->fd         = stdout;
  v->format          = VIEWER_FORMAT_ASCII_DEFAULT;
  v->iformat         = 0;
  v->outputname      = 0;
  vascii->tab        = 0;
  v->type_name    = (char *) PetscMalloc((1+PetscStrlen(ASCII_VIEWER))*sizeof(char));CHKPTRQ(v->type_name);
  PetscStrcpy(v->type_name,ASCII_VIEWER);

  ierr = PetscObjectComposeFunction((PetscObject)v,"ViewerSetFilename_C",
                                    "ViewerSetFilename_ASCII",
                                     (void*)ViewerSetFilename_ASCII);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}
EXTERN_C_END

