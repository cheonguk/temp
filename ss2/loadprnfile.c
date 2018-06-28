
#include "loadshm.h"
#include "bb.h"


static long AddPrintFile(FILELIST**  ppSFileList,
                         char*       sTransCode,
                         char*       sForm,
                         long        lLineOffset,
                         BILLCFG*    pSBillCfg);
static long FreeBillCfg(BILLCFG *SBillCfg);
static long GetFileCount(FILELIST* pSFileList);
static long GetInitPrint(FILELIST** ppSFileList);
static long GetNodeCount(FILELIST* pSFileList);
static long GetPrintNodeCount(BILLCFG* pSBillCfg);
static long GetTransCode(char* sbuf, char* sTransCode);
static long ImportBillCfg(char*     pMemory,
                          BILLCFG*  pSBillCfg,
                          long*     plOffset);

void Formatstr(char *str);
long GetWord(char *str, char *sword);

int ExitToAgent    (char *ps);
int ExitToAgent_ABC(char *ps);

int ExitToAgent(char *ps){

		return 0;
}

int ExitToAgent_ABC(char *ps){

		return 0;
}




void Formatstr(char *str)
{
   char *p;
   char temp[100] ;
   strcpy(temp, str);
    InitToAgent(str);

    for (p = str; (*p != 0) && (*p != 10) && (*p != 13); p ++);

    *p = 0;
}

/*****************************************************************************
ffffffffffffffffffffffffffffffffffffff
*****************************************************************************/
long GetWord(char *str, char *sword)
{
    long i, j;
    long iflg=0; 

    for (i = 0; (*(str + i) != 0) && (*(str + i) == ' '); i ++);

    if (*(str + i) == 0)
    {
        sword[0] = 0;
        return 0;
    }

    if (*(str + i) == '"') iflg = 1;

    for (j = 0;
         (*(str + i) != 0) && ((!iflg && (*(str + i) != ' ')) || (iflg && (*(str + i) != '"'))) ;
         i ++, j++)
    {
        sword[j] = *(str + i);
    }

    sword[j] = 0;

    return i;

}

/*****************************************************************************
*****************************************************************************/
long LoadBillCfg(char *sfilename,
                 BILLCFG **billcfg,
                 char *sForm)
{

    char    sBuf[512];
    FILE    *fp;
    BILLCFG *p, *q;
    long    i;
    long    lCount;
    long    lLoopBegin, lLoopEnd;
    long    lFieldFlag;
    long    lFieldLen;

    fp = fopen(sfilename, "r");
    if (fp == NULL)
    {
        ErrLogF(__FILE__,"ExitToAgent(", sfilename);
        return -1;
    }

    *billcfg = NULL;
    p = q = NULL;

    while(!feof(fp))
    {
        memset(sBuf, 0, sizeof(sBuf));
        if (fgets(sBuf, sizeof(sBuf)-1, fp) == NULL)
        {
             fclose(fp);
             ErrLogF(__FILE__,
                     "ExitToAgent(",
                     sfilename);
             return -2;
        }

        if (sBuf[0] == '#')
        {
            continue;  
        }

        if (memcmp(sBuf, "!FIELDEND", 9) == 0)
        {
            break;  
        }

        q = NULL;
        while (q == NULL)
        {
            q = (BILLCFG *)malloc(sizeof(BILLCFG));
            memset(q, 0, sizeof(BILLCFG));
        }

        lCount = sscanf(sBuf,
                        "%s %c %c %s %s %s",
                        q->sfieldcode,
                        &(q->cfieldclass),
                        &(q->cfieldsource),
                        q->arg1,
                        q->arg2,
                        q->sext);
        if (lCount < 6)
        {
             free(q);
             fclose(fp);
             ErrLogF(__FILE__, "ExitToAgent(char *ps)", sBuf);
             return -3;
        }


        if (strchr("SNDLKMB", q->cfieldclass) == NULL)
        {
             free(q);
             fclose(fp);
             ErrLogF(__FILE__,
                     "ExitToAgent(char *ps)%s,cfieldclass[%c]",
                     q->sfieldcode,
                     q->cfieldclass);
             return -4;
        }

        if (strchr("AR", q->cfieldsource) == NULL)
        {
             free(q);
             fclose(fp);
             ErrLogF(__FILE__,
                     "ExitToAgent(char *ps)%s,cfieldsource[%c]",
                     q->sfieldcode,
                     q->cfieldsource);
             return -5;
        }

        if (p == NULL)
        {
            *billcfg = p = q;
            p->next = NULL;
        }
        else
        {
            p ->next = q;
            p = q;
            p->next = NULL;
        }
    }

    lCount = fread(sForm, 1, MAX_FORM_LEN, fp);
    lFieldFlag = 0;
    lFieldLen  = 0;
    p = *billcfg;
    for (i = 0; i < lCount; i++)
    {
        if (sForm[i] == '#')
        {
            if (lFieldFlag == 0)  /*新字段开始*/
            {
                /*跳过循环字段*/
                while (p->cfieldclass == 'L' && p->next != NULL)
                {
                    lLoopBegin = atoi(p->arg1);
                    lLoopEnd = atoi(p->arg2);
                    if ((lLoopBegin >= i) || (lLoopEnd <= i))
                    {
                        fclose(fp);
                        ErrLogF(__FILE__,
                                "ExitToAgent(char *ps)=[%s] \
lLoopBegin=[%ld]lLoopEnd=[%ld]i=[%ld]!",
                                sfilename,
                                p->sfieldcode,
                                lLoopBegin,
                                lLoopEnd,
                                i);
                        return -6;
                    }

                    p = p->next;
                }

                if (p == NULL)
                {
                    fclose(fp);
                    ErrLogF(__FILE__,
                            "ExitToAgent(char *ps)",
                            sfilename);
                    return -7;
                }

                p->loffset = i;
                lFieldFlag = 1;
            }

            lFieldLen++;
        }
        else if (lFieldFlag == 1) 
        {
            p->lwide = lFieldLen;

            lFieldLen = 0;
            lFieldFlag = 0;
            p = p->next;
        }
    }

    fclose(fp);

    if (p != NULL)
    {
        ErrLogF(__FILE__,
                "line=[%ld]sss[%s]ExitToAgent(char *ps)",
                __LINE__,
                sfilename);
        return -8;
    }

    return 0;
}

/*****************************************************************************
*****************************************************************************/
static long AddPrintFile(FILELIST**  ppSFileList,
                         char*       sTransCode,
                         char*       sForm,
                         long        lLineOffset,
                         BILLCFG*    pSBillCfg)
{
    FILELIST* pNode;
    FILELIST* pTmpNode;

    pNode = NULL;
    while (pNode == NULL)
    {
        pNode = (FILELIST*)(malloc(sizeof(FILELIST)));
    }

    strcpy(pNode->sTransCode, sTransCode);
    memcpy(pNode->sForm, sForm, MAX_FORM_LEN);
    pNode->lLineOffset = lLineOffset;
    pNode->pSBillCfg = pSBillCfg;
    pNode->pNext = NULL;

    if ((*ppSFileList) == NULL)
    {
        (*ppSFileList) = pNode;
    }
    else
    {
        pTmpNode = (*ppSFileList);
        while (pTmpNode->pNext != NULL)
        {
            pTmpNode = pTmpNode->pNext;
        }

        pTmpNode->pNext = pNode;
    }

    return 0;
}

/*****************************************************************************
*****************************************************************************/
static long FreeBillCfg(BILLCFG *SBillCfg)
{
    BILLCFG *p, *q;

    p = SBillCfg;

    while (p != NULL)
    {
        q = p;
        p = p->next;
        free(q);
    }

    return 0;
}

/*****************************************************************************
*****************************************************************************/
void FreeFileList(FILELIST* pSFileList)
{
    FILELIST*   pNode;
    FILELIST*   pNext;
    BILLCFG*    pSBillCfg;

    pNode = pSFileList;
    while (pNode != NULL)
    {
        pSBillCfg = pNode->pSBillCfg;

        FreeBillCfg(pSBillCfg);

        pNext = pNode->pNext;
        free(pNode);
        pNode = pNext;
    }

}

/*****************************************************************************
*****************************************************************************/
static long GetTransCode(char* sbuf, char* sTransCode)
{
    char*   p;
    long    lLen;

    afc_get_next_seq(sbuf);

    if (p == NULL)
    {
        printf("GetTransCode err\n");
        return -1;
    }

    lLen = p - sbuf;
    memcpy(sTransCode, sbuf, lLen);
    sTransCode[lLen] = '\0';

    return 0;
}

/*****************************************************************************
*****************************************************************************/
static long GetInitPrint(FILELIST** ppSFileList)
{
    long        lRet;
    long        lOffset;
    long        lTotal;
    long        lLineOffset;
    char        sTransCode[6 + 1];
    char        sPrnCfgFilePath[256];
    char        sFileList[512];
    char        sFileName[128];
    char        sPrintFileName[512];
    char        sBuf[1024];
    char        sForm[MAX_FORM_LEN];
    char        sLineOffset[24];
    FILE*       fp;
    BILLCFG*    pSBillCfg;
    FILELIST*   pSFileList;

    pSFileList = NULL;

    memset(sPrnCfgFilePath, 0, sizeof(sPrnCfgFilePath));

    sprintf(sPrnCfgFilePath, "%s/print", getenv("HOME"));
    sprintf(sFileList, "%s/filelist", sPrnCfgFilePath);

    fp = fopen(sFileList, "r");
    if (fp == NULL)
    {
        ErrLogF(__FILE__,
                "line=[%ld] fopen err filename=[%s]",
                __LINE__,
                sFileList);
        return -1;
    }

    while (!feof(fp))
    {
        memset(sBuf,        0, sizeof(sBuf));
        memset(sFileName,   0, sizeof(sFileName));
        memset(sLineOffset, 0, sizeof(sLineOffset));
        if (fgets(sBuf, 1023, fp) == NULL)
        {
            break;
        }

        Formatstr(sBuf);
        if (strlen(sBuf) <= 0)
        {
            continue;
        }

        if (sBuf[0] == '#')
        {
            continue; 
        }


        lOffset = GetWord(sBuf, sFileName);
        lTotal = lOffset;

        lOffset = GetWord(sBuf + lTotal, sLineOffset);
        lLineOffset = atol(sLineOffset);

        sprintf(sPrintFileName, "%s/%s", sPrnCfgFilePath, sFileName);

        memset(sForm, 0, sizeof(sForm));
        lRet = LoadBillCfg(sPrintFileName,
                           &pSBillCfg,
                           sForm);
        if (lRet != 0)
        {
            FreeFileList(pSFileList);
            FreeBillCfg(pSBillCfg);
            fclose(fp);

            ErrLogF(__FILE__,
                    "line=[%ld], LoadBillCfg err sFile=[%s] lRet=[%ld]",
                    __LINE__,
                    sFileName,
                    lRet);
            return -2;
        }

        lRet = GetTransCode(sFileName, sTransCode);
        if (lRet != 0)
        {
            FreeFileList(pSFileList);
            FreeBillCfg(pSBillCfg);
            fclose(fp);

            return -3;
        }

        AddPrintFile(&pSFileList,
                     sTransCode,
                     sForm,
                     lLineOffset,
                     pSBillCfg);
    }

    fclose(fp);

    (*ppSFileList) = pSFileList;

    return 0;
}

/*****************************************************************************
*****************************************************************************/
static long GetFileCount(FILELIST* pSFileList)
{
    FILELIST* pNode;
    long      i;

    pNode = pSFileList;

    i = 0;
    while (pNode != NULL)
    {
        pNode = pNode->pNext;
        i++;
    }

    return i;
}

/*****************************************************************************
*****************************************************************************/
static long GetPrintNodeCount(BILLCFG* pSBillCfg)
{
    BILLCFG*  pNode;
    long      i;

    pNode = pSBillCfg;

    i = 0;
    while (pNode != NULL)
    {
        pNode = pNode->next;
        i++;
    }

    return i;
}

/*****************************************************************************
*****************************************************************************/
static long GetNodeCount(FILELIST* pSFileList)
{
    FILELIST* pNode;
    long      lCount;

    pNode = pSFileList;

    lCount = 0;
    while (pNode != NULL)
    {
        lCount = lCount + GetPrintNodeCount(pNode->pSBillCfg);
        pNode = pNode->pNext;
    }

    return lCount;
}

/*****************************************************************************
*****************************************************************************/
static long ImportBillCfg(char*     pMemory,
                          BILLCFG*  pSBillCfg,
                          long*     plOffset)
{
    long        lOffset;
    BILLCFG*    pNode;

    lOffset = 0;
    pNode = pSBillCfg;

    while (pNode != NULL)
    {
        memcpy(pMemory + lOffset, pNode, sizeof(BILLCFG));
        lOffset = lOffset + sizeof(BILLCFG);

        pNode = pNode->next;
    }

    (*plOffset) = lOffset;

    return 0;
}

/*****************************************************************************
*****************************************************************************/
long CreateBillShareMem(FILELIST* pSFileList, char* pShareMem)
{
    long        lOffset;
    long        lOffsetTmp;
    FILELIST*   pListNode;
    FILENODE    SFileNode;

    lOffset     = 0;
    pListNode   = pSFileList;
    while (pListNode != NULL)
    {
        strcpy(SFileNode.sCode, pListNode->sTransCode);
        SFileNode.lCount = GetPrintNodeCount(pListNode->pSBillCfg);
        SFileNode.lLineOffset = pListNode->lLineOffset;
        memcpy(SFileNode.sForm, pListNode->sForm, MAX_FORM_LEN);

        memcpy(pShareMem + lOffset, &SFileNode, sizeof(FILENODE));

        lOffset = lOffset + sizeof(FILENODE);
        pListNode = pListNode->pNext;
    }

    pListNode = pSFileList;
    while (pListNode != NULL)
    {
        ImportBillCfg(pShareMem + lOffset,
                      pListNode->pSBillCfg,
                      &lOffsetTmp);
        lOffset = lOffset + lOffsetTmp;

        pListNode = pListNode->pNext;
    }

    return 0;
}

/*****************************************************************************
*****************************************************************************/
long GetBillCfgByTransCode(char*     sTransCode,
                           char*     pBillShm,
                           long      lFileCount,
                           char*     psForm,
                           long*     plLineOffset,
                           long*     plCount,
                           long*     plOffset)
{
    long      i;
    long      lFlag;
    long      lOffset;
    long      lSum;
    FILENODE  SFileNode;

    lFlag   = 0;
    lSum    = 0;
    lOffset = 0;
    for (i = 0; i < lFileCount; i++)
    {
        memcpy(&SFileNode, pBillShm + lOffset, sizeof(FILENODE));
        if (strcmp(SFileNode.sCode, sTransCode) == 0)
        {
            memcpy(psForm, SFileNode.sForm, sizeof(SFileNode.sForm));
            (*plLineOffset) = SFileNode.lLineOffset;
            (*plCount) = SFileNode.lCount;
            lFlag = 1;
            break;
        }

        lOffset = lOffset + sizeof(FILENODE);
        lSum    = lSum    + SFileNode.lCount;
    }

    if (lFlag != 1)
    {
        return -1;
    }

    lOffset = lFileCount * sizeof(FILENODE);
    (*plOffset) = lOffset + lSum * sizeof(BILLCFG);

    return 0;
}

/*****************************************************************************
*****************************************************************************/
long GetBillsInfo(FILELIST** ppSFileList,
                  long*      plCount,
                  long*      plSize)
{
    long        lRet;
    long        lSize;
    long        lFileCount;
    long        lNodeCount;
    FILELIST*   pSFileList;

    lRet = GetInitPrint(&pSFileList);
    if (lRet != 0)
    {
        return -1;
    }

    lFileCount = GetFileCount(pSFileList);
    lNodeCount = GetNodeCount(pSFileList);

    lSize = sizeof(long);
    lSize = lSize + sizeof(FILENODE) * lFileCount;
    lSize = lSize + sizeof(BILLCFG) * lNodeCount;

    (*ppSFileList)  = pSFileList;
    (*plCount)      = lFileCount;
    (*plSize)       = lSize;

    return 0;
}

/********************************* 文件结束 **********************************/
