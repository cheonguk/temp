#ifndef _LOADPRNFILE_H
#define _LOADPRNFILE_H


#define MAX_FORM_LEN      2048

typedef struct tagBILLCFG{
    char    sfieldcode[20];       
    char    cfieldclass;           
    char    cfieldsource;         
    char    arg1[20];            
    char    arg2[20];            
    char    sext[5];               
    long    loffset;              
    long    lwide;                
    struct  tagBILLCFG *next;
}BILLCFG;


typedef struct tagFILELIST
{
    char        sTransCode[6 + 1];
    char        sForm[MAX_FORM_LEN];
    long        lLineOffset;
    BILLCFG*    pSBillCfg;
    struct tagFILELIST*  pNext;
}FILELIST;


typedef struct tagFILENODE
{
    char  sCode[6 + 1];
    char  sForm[MAX_FORM_LEN];
    long  lLineOffset;
    long  lCount;
}FILENODE;


/*****************************************************************************
*****************************************************************************/
long GetBillCfgByTransCode(char*     sTransCode,
                           char*     pBillShm,
                           long      lFileCount,
                           char*     psForm,
                           long*     plLineOffset,
                           long*     plCount,
                           long*     plOffset);

/*****************************************************************************
*****************************************************************************/
void FreeFileList(FILELIST* pSFileList);

/*****************************************************************************
*****************************************************************************/
long GetBillsInfo(FILELIST** ppSFileList,
                  long*      plCount,
                  long*      plSize);

/*****************************************************************************
*****************************************************************************/
long CreateBillShareMem(FILELIST* pSFileList, char* pShareMem);
long test();

#endif

