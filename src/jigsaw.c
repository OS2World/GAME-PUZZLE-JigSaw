/*---------------------------------------------------------------------------*/
/* Jigsaw for OS/2 -- 32-bit OpenWatcom port                                 */
/* Original: (c) 1988 Microsoft Corporation                                  */
/* 32-bit port: (c) 2026 OS2World                                            */
/*---------------------------------------------------------------------------*/

const char szBldLevel[] =
    "@#Microsoft Corporation:1.1#@##1## 29 Sep 2026 00:00:00      :::@@"
    " Jigsaw puzzle for OS/2 (32-bit GPI demo)";

#define INCL_BITMAPFILEFORMAT

#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR

#define INCL_DEV

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINRECTANGLES
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINSCROLLBARS
#define INCL_WINFRAMEMGR
#define INCL_WINSWITCHLIST
#define INCL_WINSYS
#define INCL_WINDIALOGS
#define INCL_WINSTDFILE
#define INCL_WINFILEDLG

#define INCL_GPIBITMAPS
#define INCL_GPICONTROL
#define INCL_GPITRANSFORMS
#define INCL_GPIPRIMITIVES
#define INCL_GPIPATHS
#define INCL_GPIREGIONS
#define INCL_GPISEGMENTS
#define INCL_GPICORRELATION
#define INCL_GPILCIDS

#define INCL_ERRORS

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "jigsaw.h"
#include "lang.h"

/*---------------------------------------------------------------------------*/
/* language table                                                             */
/*---------------------------------------------------------------------------*/

int current_lang = LANG_EN;

const char *lang_strings[LANG_COUNT][STR_COUNT] = {
  /* EN */
  { "~Game","~Load Bitmap\tCtrl+N","~Jumble\tCtrl+J","E~xit\tCtrl+X",
    "~Options","Zoom ~In\tCtrl++","Zoom ~Out\tCtrl+-","~Language",
    "~Frame Controls\tCtrl+F","~Save settings on exit","~Help","~About...",
    "OK to exit Jigsaw ?", "Yes", "No" },
  /* ES */
  { "~Juego","~Cargar Mapa\tCtrl+N","~Mezclar\tCtrl+J","~Salir\tCtrl+X",
    "~Opciones","Zoom ~Mayor\tCtrl++","Zoom ~Menor\tCtrl+-","~Idioma",
    "~Controles Marco\tCtrl+F","~Guardar ajustes al salir","~Ayuda","~Acerca de...",
    "OK para salir de Jigsaw ?", "Si", "No" },
  /* NL */
  { "~Spel","~Laad Bitmap\tCtrl+N","~Schudden\tCtrl+J","Afsl~uiten\tCtrl+X",
    "~Opties","Zoom ~In\tCtrl++","Zoom ~Uit\tCtrl+-","~Taal",
    "~Kaderbesturing\tCtrl+F","~Instellingen opslaan","~Help","~Info...",
    "Jigsaw afsluiten ?", "Ja", "Nee" },
  /* DE */
  { "~Spiel","~Bitmap laden\tCtrl+N","~Mischen\tCtrl+J","~Beenden\tCtrl+X",
    "~Optionen","Zoom ~ein\tCtrl++","Zoom ~aus\tCtrl+-","~Sprache",
    "~Rahmen\tCtrl+F","~Einstellungen speichern","~Hilfe","~Info...",
    "Jigsaw beenden ?", "Ja", "Nein" },
  /* FR */
  { "~Jeu","~Charger image\tCtrl+N","~Melanger\tCtrl+J","~Quitter\tCtrl+X",
    "~Options","Zoom ~avant\tCtrl++","Zoom ~arriere\tCtrl+-","~Langue",
    "~Cadre\tCtrl+F","~Sauver config a la sortie","~Aide","~A propos...",
    "Quitter Jigsaw ?", "Oui", "Non" },
  /* IT */
  { "~Gioco","~Carica Bitmap\tCtrl+N","~Mescola\tCtrl+J","~Esci\tCtrl+X",
    "~Opzioni","Zoom ~avanti\tCtrl++","Zoom ~indietro\tCtrl+-","~Lingua",
    "~Cornice\tCtrl+F","~Salva impostazioni all uscita","~Aiuto","~Informazioni...",
    "Uscire da Jigsaw ?", "Si", "No" },
};

/*---------------------------------------------------------------------------*/
/* settings                                                                   */
/*---------------------------------------------------------------------------*/

typedef struct { INT saveonexit; INT detaillevel; INT lang; INT zoom; } JIGSAWCFG;

static BOOL fSaveOnExit   = TRUE;
static INT  iDetailLevel  = 0;

static VOID load_settings(VOID)
{
    HFILE  hf;
    ULONG  ulAction, cbActual;
    JIGSAWCFG cfg = { 1, 0, LANG_EN, 0 };

    if (DosOpen("jigsaw.cfg", &hf, &ulAction, 0, FILE_NORMAL,
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE |
                OPEN_ACCESS_READONLY, NULL) == 0) {
        DosRead(hf, &cfg, sizeof(cfg), &cbActual);
        DosClose(hf);
    }
    fSaveOnExit   = (BOOL)cfg.saveonexit;
    iDetailLevel  = cfg.detaillevel;
    current_lang  = cfg.lang;
    if (current_lang < 0 || current_lang >= LANG_COUNT) current_lang = LANG_EN;
}

static VOID save_settings(VOID)
{
    HFILE  hf;
    ULONG  ulAction, cbActual;
    JIGSAWCFG cfg;

    cfg.saveonexit   = (INT)fSaveOnExit;
    cfg.detaillevel  = iDetailLevel;
    cfg.lang         = current_lang;
    cfg.zoom         = 0;

    DosOpen("jigsaw.cfg", &hf, &ulAction, 0, FILE_NORMAL,
            OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
            OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYWRITE |
            OPEN_ACCESS_WRITEONLY, NULL);
    DosWrite(hf, &cfg, sizeof(cfg), &cbActual);
    DosClose(hf);
}

/*---------------------------------------------------------------------------*/
/* helper macros                                                              */
/*---------------------------------------------------------------------------*/

#define ROUND_DOWN_MOD(arg,mod) \
    { if((arg)<0){ (arg)-=(mod)-1; (arg)+=(arg)%(mod); } else (arg)-=(arg)%(mod); }

#define ROUND_UP_MOD(arg,mod) \
    { if((arg)<0){ (arg)-=(arg)%(mod); } \
      else { (arg)+=(mod)-1; (arg)-=(arg)%(mod); } }

/*---------------------------------------------------------------------------*/
/* inter-thread messages                                                      */
/*---------------------------------------------------------------------------*/

#define UM_DIE        WM_USER+1
#define UM_DRAW       WM_USER+2
#define UM_VSCROLL    WM_USER+3
#define UM_HSCROLL    WM_USER+4
#define UM_SIZING     WM_USER+5
#define UM_ZOOM       WM_USER+6
#define UM_REDRAW     WM_USER+8
#define UM_JUMBLE     WM_USER+9
#define UM_LOAD       WM_USER+10
#define UM_LEFTDOWN   WM_USER+11
#define UM_MOUSEMOVE  WM_USER+12
#define UM_LEFTUP     WM_USER+13
#define UM_LAST       WM_USER+15

/*---------------------------------------------------------------------------*/
/* correlation parameters                                                     */
/*---------------------------------------------------------------------------*/

#define HITS  1L
#define DEPTH 2L

/*---------------------------------------------------------------------------*/
/* general globals                                                            */
/*---------------------------------------------------------------------------*/

HAB   habMain   = NULLHANDLE;
HMQ   hmqMain   = NULLHANDLE;
HWND  hwndFrame = NULLHANDLE;
HWND  hwndClient= NULLHANDLE;
HDC   hdcClient = NULLHANDLE;
HPS   hpsClient = NULLHANDLE;
SIZEL sizlMaxClient;
HPS   hpsPaint  = NULLHANDLE;
HRGN  hrgnInvalid = NULLHANDLE;

HAB   habAsync  = NULLHANDLE;
HMQ   hmqAsync  = NULLHANDLE;
TID   tidAsync  = 0;
#define STACKSIZE 65536
SHORT sPrty = -1;

HWND  hwndHorzScroll = NULLHANDLE;
HWND  hwndVertScroll = NULLHANDLE;
POINTS ptsScrollPos, ptsOldScrollPos;
POINTS ptsScrollMax, ptsHalfScrollMax;
POINTS ptsScrollLine;
POINTS ptsScrollPage;

#define UNITY    65536L
MATRIXLF matlfIdentity = { UNITY, 0, 0, 0, UNITY, 0, 0, 0, 1 };
LONG  lScale;
POINTL ptlScaleRef;
#define ZOOM_MAX  8
#define ZOOM_IN   1
#define ZOOM_OUT -1

POINTL ptlOffset;
POINTL ptlBotLeft   = { 0, 0 };
POINTL ptlTopRight  = { 3000, 3000 };
POINTL ptlMoveStart;
LONG   lLastSegId;
LONG   lPickedSeg   = 0L;
POINTL ptlOffStart;
RECTL  rclBounds;
POINTL ptlOldMouse  = { 0L, 0L };
BOOL   fButtonDownMain  = FALSE;
BOOL   fButtonDownAsync = FALSE;

POINTL ptlUpdtRef;
POINTL aptlUpdt[3];
BOOL   fUpdtFirst;
BOOL   fFirstLoad = TRUE;

/*---------------------------------------------------------------------------*/
/* frame controls                                                             */
/*---------------------------------------------------------------------------*/

static HWND hwndPark       = NULLHANDLE;
static BOOL fFrameShowing  = TRUE;

/*---------------------------------------------------------------------------*/
/* segment list                                                               */
/*---------------------------------------------------------------------------*/

typedef struct _SEGLIST {
    LONG            lSegId;
    struct _SEGLIST *pslPrev;
    struct _SEGLIST *pslNext;
    POINTL          ptlLocation;
    RECTL           rclCurrent;
    RECTL           rclBitBlt;
    POINTL          aptlBitBlt[4];
    LONG            lAdjacent[8];
    struct _SEGLIST *pslNextIsland;
    BOOL            fIslandMark;
    POINTL          aptlSides[12];
    HDC             hdcHole;
    HPS             hpsHole;
    HBITMAP         hbmHole;
    HDC             hdcFill;
    HPS             hpsFill;
    HBITMAP         hbmFill;
    POINTL          ptlModelXlate;
} SEGLIST;
typedef SEGLIST  *PSEGLIST;
typedef PSEGLIST *PPSEGLIST;

PSEGLIST pslHead   = NULL;
PSEGLIST pslTail   = NULL;
PSEGLIST pslPicked = NULL;
#define ADD_HEAD_SEG  1
#define ADD_TAIL_SEG  2
#define DEL_SEG       3
#define MAKE_TAIL_SEG 4

/*---------------------------------------------------------------------------*/
/* load info                                                                  */
/*---------------------------------------------------------------------------*/

typedef struct { CHAR szFileName[CCHMAXPATH]; } LOADINFO;
typedef LOADINFO *PLOADINFO;

/*---------------------------------------------------------------------------*/
/* bitmap data                                                                */
/*---------------------------------------------------------------------------*/

HPS     hpsBitmapFile = NULLHANDLE;
HDC     hdcBitmapFile = NULLHANDLE;
HBITMAP hbmBitmapFile = NULLHANDLE;
BITMAPINFOHEADER bmpBitmapFile = { 12L, 0, 0, 0, 0 };

HPS     hpsBitmapSize = NULLHANDLE;
HDC     hdcBitmapSize = NULLHANDLE;
HBITMAP hbmBitmapSize = NULLHANDLE;

HPS     hpsBitmapBuff = NULLHANDLE;
HDC     hdcBitmapBuff = NULLHANDLE;
HBITMAP hbmBitmapBuff = NULLHANDLE;

HPS     hpsBitmapSave = NULLHANDLE;
HDC     hdcBitmapSave = NULLHANDLE;
HBITMAP hbmBitmapSave = NULLHANDLE;
BITMAPINFOHEADER bmpBitmapSave = { 12L, 0, 0, 0, 0 };

DEVOPENSTRUC dop = { NULL, "DISPLAY", NULL, NULL, NULL, NULL, NULL, NULL, NULL };

/*---------------------------------------------------------------------------*/
/* old-style bitmap header                                                    */
/*---------------------------------------------------------------------------*/

typedef struct {
    USHORT wType;
    ULONG  dwSize;
    int    xHotspot;
    int    yHotspot;
    ULONG  dwBitsOffset;
    USHORT bmWidth;
    USHORT bmHeight;
    USHORT bmPlanes;
    USHORT bmBitcount;
} RCBITMAP;
typedef RCBITMAP *PRCBITMAP;

/*---------------------------------------------------------------------------*/
/* miscellaneous                                                              */
/*---------------------------------------------------------------------------*/

volatile BOOL fTerminate = FALSE;  /* async thread should die     */
volatile BOOL fDrawOn    = FALSE;  /* drawing enabled             */
volatile BOOL fLoadMsg   = FALSE;  /* load in progress            */

PSZ   pszLoadMsg = "Patience, preparing puzzle pieces ...";
SWCNTRL swctl = { 0, 0, 0, 0, 0, SWL_VISIBLE, SWL_JUMPABLE, "", 0 };
HSWITCH hsw;
CHAR  szTitle[80];

BOOL  fErrMem = FALSE;
LONG  lByteAlignX, lByteAlignY;

typedef struct { PSZ pszMsg; } CONFIRMPARAM;

/*---------------------------------------------------------------------------*/
/* function prototypes                                                        */
/*---------------------------------------------------------------------------*/

VOID     CalcBounds(VOID);
VOID     CalcSize(MPARAM, MPARAM);
VOID     CalcTransform(HWND);
MRESULT EXPENTRY ClientWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY AboutDlgProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY ConfirmDlgProc(HWND, ULONG, MPARAM, MPARAM);
BOOL     CreateBitmapHdcHps(HDC *, HPS *);
BOOL     CreateThread(VOID);
BOOL     CreatePicture(VOID);
BOOL     DoDraw(HPS, HRGN, BOOL);
VOID     DoHorzScroll(VOID);
VOID     DoVertScroll(VOID);
VOID     DrawPiece(HPS, PSEGLIST, BOOL);
BOOL     DumpPicture(VOID);
VOID     Finalize(VOID);
BOOL     Initialize(VOID);
VOID     Jumble(VOID);
VOID     LeftDown(MPARAM);
VOID     LeftUp(VOID);
VOID     Load(PLOADINFO);
int      main(void);
VOID     MarkIsland(PSEGLIST, BOOL);
VOID     MyMessageBox(HWND, PSZ);
VOID     MouseMove(MPARAM);
VOID APIENTRY NewThread(ULONG ulParam);
BOOL     PrepareBitmap(VOID);
BOOL     ReadBitmap(PCHAR);
VOID     Redraw(VOID);
VOID     ReportError(HAB);
BOOL     SegListCheck(INT);
PSEGLIST SegListGet(LONG);
PSEGLIST SegListUpdate(USHORT, PSEGLIST);
BOOL     SendCommand(ULONG, MPARAM, MPARAM);
VOID     SetDVTransform(FIXED, FIXED, FIXED, FIXED, LONG, LONG, LONG);
VOID     SetRect(PSEGLIST);
VOID     ShowMessage(HPS, PSZ);
VOID     ToggleMenuItem(USHORT, USHORT, PBOOL);
MRESULT  WndProcCommand(HWND, ULONG, MPARAM, MPARAM);
MRESULT  WndProcCreate(HWND);
MRESULT  WndProcPaint(VOID);
MRESULT  WndProcSize(MPARAM, MPARAM);
VOID     Zoom(SHORT);
VOID     ZoomMenuItems(VOID);
VOID     set_language(INT);
VOID     toggle_frame_controls(VOID);

/*===========================================================================*/

VOID
MyMessageBox(HWND hWnd, PSZ psz)
{
    WinMessageBox(HWND_DESKTOP, hWnd, psz, szTitle, 0,
                  MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
}

int
main(void)
{
    QMSG qmsg;

    load_settings();

    if (Initialize()) {
        set_language(current_lang);
        while (WinGetMsg(habMain, &qmsg, NULLHANDLE, 0, 0))
            WinDispatchMsg(habMain, &qmsg);
    }
    else
        ReportError(habMain);

    if (fSaveOnExit)
        save_settings();

    Finalize();
    return 0;
}

BOOL
Initialize(VOID)
{
    ULONG flCreate;
    PID   pid;
    TID   tid;
    LONG  cxScr, cyScr, cxWin, cyWin;

    WinShowPointer(HWND_DESKTOP, TRUE);
    habMain = WinInitialize(0);
    if (!habMain) return FALSE;

    hmqMain = WinCreateMsgQueue(habMain, 0);
    if (!hmqMain) return FALSE;

    WinLoadString(habMain, NULLHANDLE, IDS_TITLE, sizeof(szTitle), szTitle);

    if (!WinRegisterClass(habMain, (PCH)szTitle, (PFNWP)ClientWndProc,
                          CS_SIZEREDRAW, 0))
        return FALSE;

    flCreate = (FCF_STANDARD | FCF_VERTSCROLL | FCF_HORZSCROLL)
               & ~(ULONG)FCF_TASKLIST;

    hwndFrame = WinCreateStdWindow(HWND_DESKTOP, 0L, &flCreate,
                                   szTitle, szTitle, 0L,
                                   NULLHANDLE, ID_RESOURCE, &hwndClient);
    if (!hwndFrame) return FALSE;

    WinQueryWindowProcess(hwndFrame, &pid, &tid);
    swctl.hwnd      = hwndFrame;
    swctl.idProcess = pid;
    strcpy(swctl.szSwtitle, szTitle);
    hsw = WinAddSwitchEntry(&swctl);

    ZoomMenuItems();

    /* check "Save settings on exit" in menu if enabled */
    if (fSaveOnExit) {
        BOOL fTemp = FALSE; /* ToggleMenuItem toggles, so start FALSE to get TRUE */
        ToggleMenuItem(IDM_SUBMENU_OPTIONS, IDM_SAVEONEXIT, &fTemp);
        fSaveOnExit = TRUE;
    }

    if (!CreateThread()) return FALSE;
    if (!CreateBitmapHdcHps(&hdcBitmapFile, &hpsBitmapFile)) return FALSE;
    if (!CreateBitmapHdcHps(&hdcBitmapSize, &hpsBitmapSize)) return FALSE;
    if (!CreateBitmapHdcHps(&hdcBitmapBuff, &hpsBitmapBuff)) return FALSE;
    if (!CreateBitmapHdcHps(&hdcBitmapSave, &hpsBitmapSave)) return FALSE;

    cxScr = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
    cyScr = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
    cxWin = (1024 < cxScr) ? 1024 : cxScr;
    cyWin = ( 768 < cyScr) ?  768 : cyScr;
    WinSetWindowPos(hwndFrame, HWND_TOP,
                    (cxScr - cxWin) / 2, (cyScr - cyWin) / 2,
                    cxWin, cyWin,
                    SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ACTIVATE);

    return TRUE;
}

VOID
Finalize(VOID)
{
    if (tidAsync) {
        fDrawOn    = FALSE;
        fTerminate = TRUE;
    }

    while (pslHead != NULL) {
        GpiSetBitmap(pslHead->hpsFill, NULLHANDLE);
        GpiDeleteBitmap(pslHead->hbmFill);
        GpiDestroyPS(pslHead->hpsFill);
        DevCloseDC(pslHead->hdcFill);

        GpiSetBitmap(pslHead->hpsHole, NULLHANDLE);
        GpiDeleteBitmap(pslHead->hbmHole);
        GpiDestroyPS(pslHead->hpsHole);
        DevCloseDC(pslHead->hdcHole);

        SegListUpdate(DEL_SEG, pslHead);
    }
    if (lLastSegId > 0)
        GpiDeleteSegments(hpsClient, 1L, lLastSegId);

    if (hrgnInvalid)
        GpiDestroyRegion(hpsClient, hrgnInvalid);
    if (hpsClient) {
        GpiAssociate(hpsClient, NULLHANDLE);
        GpiDestroyPS(hpsClient);
    }
    if (hpsPaint)
        GpiDestroyPS(hpsPaint);

    if (hpsBitmapFile) {
        GpiSetBitmap(hpsBitmapFile, NULLHANDLE);
        GpiDeleteBitmap(hbmBitmapFile);
        GpiDestroyPS(hpsBitmapFile);
        DevCloseDC(hdcBitmapFile);
    }
    if (hpsBitmapSize) {
        GpiSetBitmap(hpsBitmapSize, NULLHANDLE);
        GpiDeleteBitmap(hbmBitmapSize);
        GpiDestroyPS(hpsBitmapSize);
        DevCloseDC(hdcBitmapSize);
    }
    if (hpsBitmapBuff) {
        GpiSetBitmap(hpsBitmapBuff, NULLHANDLE);
        GpiDeleteBitmap(hbmBitmapBuff);
        GpiDestroyPS(hpsBitmapBuff);
        DevCloseDC(hdcBitmapBuff);
    }
    if (hpsBitmapSave) {
        GpiSetBitmap(hpsBitmapSave, NULLHANDLE);
        GpiDeleteBitmap(hbmBitmapSave);
        GpiDestroyPS(hpsBitmapSave);
        DevCloseDC(hdcBitmapSave);
    }
    if (hwndPark)
        WinDestroyWindow(hwndPark);
    if (hwndFrame)
        WinDestroyWindow(hwndFrame);
    if (hmqMain)
        WinDestroyMsgQueue(hmqMain);
    if (habMain)
        WinTerminate(habMain);
}

VOID
ReportError(HAB hab)
{
    PERRINFO perriBlk;
    PSZ      pszErrMsg;
    PUSHORT  pusOff;

    if (!hwndFrame) return;
    if (!fErrMem) {
        perriBlk = WinGetErrorInfo(hab);
        if (!perriBlk) return;
        pusOff     = (PUSHORT)((PBYTE)perriBlk + perriBlk->offaoffszMsg);
        pszErrMsg  = (PSZ)   ((PBYTE)perriBlk + pusOff[0]);
        WinMessageBox(HWND_DESKTOP, hwndFrame, pszErrMsg, szTitle, 0,
                      MB_CUACRITICAL | MB_ENTER);
        WinFreeErrorInfo(perriBlk);
    } else {
        WinMessageBox(HWND_DESKTOP, hwndFrame,
                      "ERROR - Out Of Memory", szTitle, 0,
                      MB_CUACRITICAL | MB_ENTER);
    }
}

BOOL
CreateThread(VOID)
{
    if (DosCreateThread(&tidAsync, (PFNTHREAD)NewThread, 0, 0, STACKSIZE)) {
        fErrMem = TRUE;
        return FALSE;
    }
    return TRUE;
}

BOOL
SendCommand(ULONG ulCommand, MPARAM mp1, MPARAM mp2)
{
    if (!hmqAsync) return FALSE;

    switch (ulCommand) {
        case UM_DIE:
        case UM_LEFTDOWN:
        case UM_LEFTUP:
        case UM_MOUSEMOVE:
        case UM_DRAW:
        case UM_HSCROLL:
        case UM_VSCROLL:
        case UM_ZOOM:
        case UM_REDRAW:
        case UM_SIZING:
        case UM_JUMBLE:
        case UM_LOAD:
            return WinPostQueueMsg(hmqAsync, ulCommand, mp1, mp2);
        default:
            return TRUE;
    }
}

MRESULT EXPENTRY
ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg) {
        case WM_CREATE:
            return WndProcCreate(hwnd);

        case WM_CLOSE:
        {
            CONFIRMPARAM cp;
            cp.pszMsg = tr(STR_EXIT_CONFIRM);
            if (WinDlgBox(HWND_DESKTOP, hwndFrame, ConfirmDlgProc,
                          NULLHANDLE, IDD_CONFIRM, &cp) == MBID_YES)
                WinPostMsg(hwnd, WM_QUIT, (MPARAM)0, (MPARAM)0);
            break;
        }

        case WM_PAINT:
            return WndProcPaint();

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_MINMAXFRAME:
            if ((((PSWP)mp1)->fl & SWP_RESTORE) ||
                (((PSWP)mp1)->fl & SWP_MAXIMIZE)) {
                fDrawOn = FALSE;
                SendCommand(UM_SIZING, 0, 0);
            }
            break;

        case WM_COMMAND:
            return WndProcCommand(hwnd, msg, mp1, mp2);

        case WM_HSCROLL:
            SendCommand(UM_HSCROLL, mp2, 0);
            break;

        case WM_VSCROLL:
            SendCommand(UM_VSCROLL, mp2, 0);
            break;

        case WM_SIZE:
            fDrawOn = FALSE;
            SendCommand(UM_SIZING, mp1, mp2);
            break;

        case WM_BUTTON1DBLCLK:
        case WM_BUTTON1DOWN:
            if (hwnd != WinQueryFocus(HWND_DESKTOP))
                WinSetFocus(HWND_DESKTOP, hwnd);
            if (!fButtonDownMain) {
                fButtonDownMain = TRUE;
                SendCommand(UM_LEFTDOWN, mp1, 0);
            }
            return (MRESULT)TRUE;

        case WM_BUTTON1UP:
            if (!fButtonDownMain) return (MRESULT)TRUE;
            if (SendCommand(UM_LEFTUP, mp1, 0))
                fButtonDownMain = FALSE;
            else
                WinAlarm(HWND_DESKTOP, WA_WARNING);
            return (MRESULT)TRUE;

        case WM_MOUSEMOVE:
            if (fButtonDownMain)
                SendCommand(UM_MOUSEMOVE, mp1, 0);
            return WinDefWindowProc(hwnd, msg, mp1, mp2);

        default:
            return WinDefWindowProc(hwnd, msg, mp1, mp2);
    }
    return (MRESULT)FALSE;
}

MRESULT
WndProcCreate(HWND hwnd)
{
    SIZEL sizlPickApp;

    sizlMaxClient.cx = WinQuerySysValue(HWND_DESKTOP, SV_CXFULLSCREEN);
    sizlMaxClient.cy = WinQuerySysValue(HWND_DESKTOP, SV_CYFULLSCREEN);

    lByteAlignX = WinQuerySysValue(HWND_DESKTOP, SV_CXBYTEALIGN);
    lByteAlignY = WinQuerySysValue(HWND_DESKTOP, SV_CYBYTEALIGN);

    hdcClient = WinOpenWindowDC(hwnd);
    hpsClient = GpiCreatePS(habMain, hdcClient, &sizlMaxClient,
                             GPIA_ASSOC | PU_PELS);
    if (!hpsClient) return (MRESULT)TRUE;

    GpiSetAttrMode(hpsClient, AM_PRESERVE);

    hwndHorzScroll = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
                                     FID_HORZSCROLL);
    hwndVertScroll = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
                                     FID_VERTSCROLL);

    hpsPaint = GpiCreatePS(habMain, NULLHANDLE, &sizlMaxClient, PU_PELS);

    hrgnInvalid = GpiCreateRegion(hpsClient, 0L, NULL);

    sizlPickApp.cx = sizlPickApp.cy = 1;
    GpiSetPickApertureSize(hpsClient, PICKAP_REC, &sizlPickApp);
    return (MRESULT)FALSE;
}

MRESULT
WndProcPaint(VOID)
{
    HRGN  hrgnUpdt;
    SHORT sRgnType;

    hrgnUpdt  = GpiCreateRegion(hpsPaint, 0L, NULL);
    sRgnType  = WinQueryUpdateRegion(hwndClient, hrgnUpdt);
    SendCommand(UM_DRAW, (MPARAM)hrgnUpdt, 0);
    if (fLoadMsg)
        ShowMessage(hpsClient, pszLoadMsg);
    WinValidateRegion(hwndClient, hrgnUpdt, FALSE);
    return (MRESULT)FALSE;
}

MRESULT
WndProcCommand(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PLOADINFO  pli;
    FILEDLG    fdlg;
    INT        iCmd;

    iCmd = (INT)SHORT1FROMMP(mp1);

    switch (iCmd) {
        case IDM_JUMBLE:
            fDrawOn = FALSE;
            SendCommand(UM_JUMBLE, 0, 0);
            break;

        case IDM_LOAD:
            if (DosAllocMem((PPVOID)&pli, sizeof(LOADINFO),
                            PAG_READ | PAG_WRITE | PAG_COMMIT) != 0)
                break;
            memset(&fdlg, 0, sizeof(fdlg));
            fdlg.cbSize   = sizeof(FILEDLG);
            fdlg.fl       = FDS_OPEN_DIALOG | FDS_CENTER;
            fdlg.pszTitle = "Load Bitmap";
            strcpy(fdlg.szFullFile, "*.bmp");
            if (WinFileDlg(HWND_DESKTOP, hwnd, &fdlg) &&
                fdlg.lReturn == DID_OK) {
                strcpy(pli->szFileName, fdlg.szFullFile);
                fDrawOn = FALSE;
                SendCommand(UM_LOAD, (MPARAM)pli, 0);
            } else {
                DosFreeMem(pli);
            }
            break;

        case IDM_EXIT:
        {
            CONFIRMPARAM cp;
            cp.pszMsg = tr(STR_EXIT_CONFIRM);
            if (WinDlgBox(HWND_DESKTOP, hwndFrame, ConfirmDlgProc,
                          NULLHANDLE, IDD_CONFIRM, &cp) == MBID_YES)
                WinPostMsg(hwnd, WM_QUIT, (MPARAM)0, (MPARAM)0);
            break;
        }

        case IDM_ZOOMIN:
            fDrawOn = FALSE;
            SendCommand(UM_ZOOM, (MPARAM)ZOOM_IN, 0);
            break;

        case IDM_ZOOMOUT:
            fDrawOn = FALSE;
            SendCommand(UM_ZOOM, (MPARAM)ZOOM_OUT, 0);
            break;

        case IDM_FRAME:
            toggle_frame_controls();
            break;

        case IDM_SAVEONEXIT:
            ToggleMenuItem(IDM_SUBMENU_OPTIONS, IDM_SAVEONEXIT, &fSaveOnExit);
            break;

        case IDM_LANG_EN:
        case IDM_LANG_ES:
        case IDM_LANG_NL:
        case IDM_LANG_DE:
        case IDM_LANG_FR:
        case IDM_LANG_IT:
            set_language(iCmd - IDM_LANG_EN);
            break;

        case IDM_ABOUT:
            WinDlgBox(HWND_DESKTOP, hwnd, AboutDlgProc,
                      NULLHANDLE, IDD_ABOUT, (MPARAM)0);
            break;

        default:
            return WinDefWindowProc(hwnd, msg, mp1, mp2);
    }
    return (MRESULT)FALSE;
}

MRESULT EXPENTRY
AboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg) {
        case WM_COMMAND:
            if (SHORT1FROMMP(mp1) == DID_OK)
                WinDismissDlg(hwnd, DID_OK);
            break;
        default:
            return WinDefDlgProc(hwnd, msg, mp1, mp2);
    }
    return (MRESULT)FALSE;
}

MRESULT EXPENTRY
ConfirmDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg) {
        case WM_INITDLG:
        {
            CONFIRMPARAM *p = (CONFIRMPARAM *)PVOIDFROMMP(mp2);
            if (p) WinSetDlgItemText(hwnd, IDC_CONFIRM_MSG, p->pszMsg);
            WinSetDlgItemText(hwnd, IDC_YES, tr(STR_YES));
            WinSetDlgItemText(hwnd, IDC_NO,  tr(STR_NO));
            WinSetWindowText(hwnd, szTitle);
            return (MRESULT)FALSE;
        }
        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1)) {
                case IDC_YES: WinDismissDlg(hwnd, MBID_YES); break;
                case IDC_NO:  WinDismissDlg(hwnd, MBID_NO);  break;
            }
            break;
        case WM_SYSCOMMAND:
            if (SHORT1FROMMP(mp1) == SC_CLOSE)
                WinDismissDlg(hwnd, MBID_NO);
            break;
        default:
            return WinDefDlgProc(hwnd, msg, mp1, mp2);
    }
    return (MRESULT)FALSE;
}

VOID
set_language(INT lang)
{
    HWND     hwndMenu, hwndGameSub, hwndOptSub, hwndLangSub, hwndHelpSub;
    MENUITEM mi;
    INT      i;

    if (lang < 0 || lang >= LANG_COUNT) return;
    current_lang = lang;

    hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);
    if (!hwndMenu) return;

    /* get submenu handles */
    WinSendMsg(hwndMenu, MM_QUERYITEM,
               MPFROM2SHORT(IDM_SUBMENU_GAME, FALSE), MPFROMP(&mi));
    hwndGameSub = mi.hwndSubMenu;

    WinSendMsg(hwndMenu, MM_QUERYITEM,
               MPFROM2SHORT(IDM_SUBMENU_OPTIONS, FALSE), MPFROMP(&mi));
    hwndOptSub = mi.hwndSubMenu;

    WinSendMsg(hwndOptSub, MM_QUERYITEM,
               MPFROM2SHORT(IDM_SUBMENU_LANG, FALSE), MPFROMP(&mi));
    hwndLangSub = mi.hwndSubMenu;

    WinSendMsg(hwndMenu, MM_QUERYITEM,
               MPFROM2SHORT(IDM_SUBMENU_HELP, FALSE), MPFROMP(&mi));
    hwndHelpSub = mi.hwndSubMenu;

    /* update menu bar top-level text */
    WinSendMsg(hwndMenu, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_SUBMENU_GAME), MPFROMP(tr(STR_MENU_GAME)));
    WinSendMsg(hwndMenu, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_SUBMENU_OPTIONS), MPFROMP(tr(STR_MENU_OPTIONS)));
    WinSendMsg(hwndMenu, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_SUBMENU_HELP), MPFROMP(tr(STR_MENU_HELP)));

    /* update Game submenu items */
    WinSendMsg(hwndGameSub, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_LOAD), MPFROMP(tr(STR_MENU_LOAD)));
    WinSendMsg(hwndGameSub, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_JUMBLE), MPFROMP(tr(STR_MENU_JUMBLE)));
    WinSendMsg(hwndGameSub, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_EXIT), MPFROMP(tr(STR_MENU_EXIT)));

    /* update Options submenu items */
    WinSendMsg(hwndOptSub, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_ZOOMIN), MPFROMP(tr(STR_MENU_ZOOMIN)));
    WinSendMsg(hwndOptSub, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_ZOOMOUT), MPFROMP(tr(STR_MENU_ZOOMOUT)));
    WinSendMsg(hwndOptSub, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_SUBMENU_LANG), MPFROMP(tr(STR_MENU_LANGUAGE)));
    WinSendMsg(hwndOptSub, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_FRAME), MPFROMP(tr(STR_MENU_FRAME)));
    WinSendMsg(hwndOptSub, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_SAVEONEXIT), MPFROMP(tr(STR_MENU_SAVEONEXIT)));

    /* update Help submenu items */
    WinSendMsg(hwndHelpSub, MM_SETITEMTEXT,
               MPFROMSHORT(IDM_ABOUT), MPFROMP(tr(STR_MENU_ABOUT)));

    /* update language checkmarks */
    for (i = 0; i < LANG_COUNT; i++)
        WinSendMsg(hwndLangSub, MM_SETITEMATTR,
                   MPFROM2SHORT(IDM_LANG_EN + i, TRUE),
                   MPFROM2SHORT(MIA_CHECKED, (i == lang) ? MIA_CHECKED : 0));
}

VOID
toggle_frame_controls(VOID)
{
    if (fFrameShowing) {
        if (!hwndPark)
            hwndPark = WinCreateWindow(HWND_OBJECT, WC_FRAME, "",
                                       0, 0, 0, 0, 0,
                                       NULLHANDLE, HWND_TOP, 0, NULL, NULL);
        WinSetParent(hwndHorzScroll, hwndPark, FALSE);
        WinSetParent(hwndVertScroll, hwndPark, FALSE);
        WinSetParent(WinWindowFromID(hwndFrame, FID_MENU), hwndPark, FALSE);
        fFrameShowing = FALSE;
    } else {
        WinSetParent(hwndHorzScroll, hwndFrame, FALSE);
        WinSetParent(hwndVertScroll, hwndFrame, FALSE);
        WinSetParent(WinWindowFromID(hwndPark, FID_MENU), hwndFrame, FALSE);
        fFrameShowing = TRUE;
    }
    WinSendMsg(hwndFrame, WM_UPDATEFRAME,
               MPFROMLONG(FCF_MENU | FCF_HORZSCROLL | FCF_VERTSCROLL),
               (MPARAM)0);
}

VOID
ShowMessage(HPS hps, PSZ psz)
{
    HRGN   hrgn;
    RECTL  rclClient;
    POINTL ptl;

    WinQueryWindowRect(hwndClient, &rclClient);
    hrgn = GpiCreateRegion(hps, 1L, &rclClient);
    GpiSetColor(hps, CLR_BACKGROUND);
    GpiPaintRegion(hps, hrgn);
    GpiDestroyRegion(hps, hrgn);

    GpiSetColor(hps, CLR_NEUTRAL);
    ptl.x = 0L;
    ptl.y = 0L;
    GpiMove(hps, &ptl);
    GpiCharString(hps, (LONG)strlen(psz), psz);
}

VOID
Load(PLOADINFO pli)
{
    RECTL rclClient;

    ShowMessage(hpsClient, pszLoadMsg);
    fLoadMsg = TRUE;

    if (!fFirstLoad) DumpPicture();

    if (!ReadBitmap(pli->szFileName)) {
        MyMessageBox(hwndClient, "Error reading file.");
        fLoadMsg = FALSE;
        DosFreeMem(pli);
        return;
    }
    if (!PrepareBitmap()) {
        MyMessageBox(hwndClient, "Error preparing bitmap.");
        fLoadMsg = FALSE;
        DosFreeMem(pli);
        return;
    }

    strcpy(swctl.szSwtitle, szTitle);
    strcat(swctl.szSwtitle, ": ");
    strncat(swctl.szSwtitle, pli->szFileName,
            sizeof(swctl.szSwtitle) - strlen(swctl.szSwtitle) - 1);
    WinChangeSwitchEntry(hsw, &swctl);
    WinSetWindowText(hwndFrame, swctl.szSwtitle);

    WinQueryWindowRect(hwndClient, &rclClient);
    ptsScrollMax.x      = (SHORT)(rclClient.xRight - rclClient.xLeft);
    ptsHalfScrollMax.x  = ptsScrollMax.x >> 1;
    ptsScrollPage.x     = ptsScrollMax.x >> 3;
    ROUND_DOWN_MOD(ptsScrollPage.x, (SHORT)lByteAlignX);
    ptsScrollLine.x     = ptsScrollMax.x >> 5;
    ROUND_DOWN_MOD(ptsScrollLine.x, (SHORT)lByteAlignX);
    ptsScrollPos.x      = ptsHalfScrollMax.x;
    ptsOldScrollPos.x   = ptsHalfScrollMax.x;
    WinSendMsg(hwndHorzScroll, SBM_SETSCROLLBAR,
               MPFROMSHORT(ptsScrollPos.x),
               MPFROM2SHORT(1, ptsScrollMax.x));

    ptsScrollMax.y      = (SHORT)(rclClient.yTop - rclClient.yBottom);
    ptsHalfScrollMax.y  = ptsScrollMax.y >> 1;
    ptsScrollPage.y     = ptsScrollMax.y >> 3;
    ROUND_DOWN_MOD(ptsScrollPage.y, (SHORT)lByteAlignY);
    ptsScrollLine.y     = ptsScrollMax.y >> 5;
    ROUND_DOWN_MOD(ptsScrollLine.y, (SHORT)lByteAlignY);
    ptsScrollPos.y      = ptsHalfScrollMax.y;
    ptsOldScrollPos.y   = ptsHalfScrollMax.y;
    WinSendMsg(hwndVertScroll, SBM_SETSCROLLBAR,
               MPFROMSHORT(ptsScrollPos.y),
               MPFROM2SHORT(1, ptsScrollMax.y));

    CreatePicture();
    lScale = 0;
    CalcBounds();
    ptlScaleRef.x = ptlScaleRef.y = 0L;
    CalcTransform(hwndClient);
    DosFreeMem(pli);
    fFirstLoad = FALSE;
    fLoadMsg   = FALSE;
}

VOID
Jumble(VOID)
{
    LONG      lWidth, lHeight;
    DATETIME  date;
    POINTL    ptl;
    RECTL     rclClient;
    PSEGLIST  psl;
    MATRIXLF  matlf;

    if (!WinQueryWindowRect(hwndClient, &rclClient)) return;
    lWidth  = rclClient.xRight - rclClient.xLeft;
    lHeight = rclClient.yTop   - rclClient.yBottom;
    if ((lWidth <= 0) || (lHeight <= 0)) return;

    matlf = matlfIdentity;
    DosGetDateTime(&date);
    srand((USHORT)date.hundredths);
    for (psl = pslHead; psl != NULL; psl = psl->pslNext) {
        psl->pslNextIsland = psl;
        psl->fIslandMark   = FALSE;
        ptl.x = rclClient.xLeft   + (rand() % lWidth);
        ptl.y = rclClient.yBottom + (rand() % lHeight);
        GpiConvert(hpsClient, CVTC_DEVICE, CVTC_MODEL, 1L, &ptl);
        ptl.x = 50 * (ptl.x / 50) - 250;
        ptl.y = 50 * (ptl.y / 50) - 250;
        matlf.lM31 = ptl.x - psl->ptlLocation.x;
        matlf.lM32 = ptl.y - psl->ptlLocation.y;
        GpiSetSegmentTransformMatrix(hpsClient, psl->lSegId, 9L,
                                     &matlf, TRANSFORM_REPLACE);
        psl->ptlModelXlate.x = matlf.lM31;
        psl->ptlModelXlate.y = matlf.lM32;
        SetRect(psl);
    }
}

VOID APIENTRY
NewThread(ULONG ulParam)
{
    QMSG   qmsgAsync, qmsgPeek;
    BOOL   fDone;
    SHORT  sZoomArg;
    POINTL aptlDraw[3];

    (VOID)ulParam;

    habAsync = WinInitialize(0);
    if (!habAsync) {
        WinPostMsg(hwndClient, WM_QUIT, (MPARAM)0, (MPARAM)0);
        DosExit(EXIT_THREAD, 0);
    }

    hmqAsync = WinCreateMsgQueue(habAsync, 150);
    if (!hmqAsync) {
        WinPostMsg(hwndClient, WM_QUIT, (MPARAM)0, (MPARAM)0);
        WinTerminate(habAsync);
        DosExit(EXIT_THREAD, 0);
    }

    DosSetPriority(PRTYS_THREAD, PRTYC_NOCHANGE, sPrty, 0);

    while (TRUE) {
        WinGetMsg(habAsync, &qmsgAsync, NULLHANDLE, 0, 0);

        if (WinPeekMsg(habAsync, &qmsgPeek, NULLHANDLE,
                       UM_DIE, UM_DIE, PM_NOREMOVE))
            qmsgAsync = qmsgPeek;

        if (WinPeekMsg(habAsync, &qmsgPeek, NULLHANDLE,
                       UM_SIZING, UM_LOAD, PM_NOREMOVE))
            fDrawOn = FALSE;
        else
            fDrawOn = TRUE;

        switch (qmsgAsync.msg) {
            case UM_LOAD:
                Load((PLOADINFO)qmsgAsync.mp1);
                Redraw();
                break;

            case UM_JUMBLE:
                Jumble();
                Redraw();
                break;

            case UM_REDRAW:
                Redraw();
                break;

            case UM_DRAW:
                if (qmsgAsync.mp1) {
                    DoDraw(hpsBitmapBuff, (HRGN)qmsgAsync.mp1, TRUE);
                    GpiQueryRegionBox(hpsClient, (HRGN)qmsgAsync.mp1,
                                      (PRECTL)aptlDraw);
                    GpiDestroyRegion(hpsClient, (HRGN)qmsgAsync.mp1);
                    WinMapWindowPoints(hwndClient, HWND_DESKTOP, aptlDraw, 3);
                    ROUND_DOWN_MOD(aptlDraw[0].x, lByteAlignX);
                    ROUND_DOWN_MOD(aptlDraw[0].y, lByteAlignY);
                    ROUND_UP_MOD(  aptlDraw[1].x, lByteAlignX);
                    ROUND_UP_MOD(  aptlDraw[1].y, lByteAlignY);
                    WinMapWindowPoints(HWND_DESKTOP, hwndClient, aptlDraw, 3);
                    aptlDraw[2] = aptlDraw[0];
                    GpiBitBlt(hpsClient, hpsBitmapBuff, 3L, aptlDraw,
                              ROP_SRCCOPY, BBO_IGNORE);
                }
                break;

            case UM_HSCROLL:
                switch (SHORT2FROMMP(qmsgAsync.mp1)) {
                    case SB_LINEUP:
                        ptsScrollPos.x -= ptsScrollLine.x; break;
                    case SB_LINEDOWN:
                        ptsScrollPos.x += ptsScrollLine.x; break;
                    case SB_SLIDERTRACK:
                    case SB_SLIDERPOSITION:
                        for (fDone = FALSE; !fDone;) {
                            if (WinPeekMsg(habAsync, &qmsgPeek, NULLHANDLE,
                                           UM_HSCROLL, UM_HSCROLL, PM_NOREMOVE))
                                if ((SHORT2FROMMP(qmsgPeek.mp1) == SB_SLIDERTRACK) ||
                                    (SHORT2FROMMP(qmsgPeek.mp1) == SB_SLIDERPOSITION))
                                    WinPeekMsg(habAsync, &qmsgAsync, NULLHANDLE,
                                               UM_HSCROLL, UM_HSCROLL, PM_REMOVE);
                                else fDone = TRUE;
                            else fDone = TRUE;
                        }
                        ptsScrollPos.x = SHORT1FROMMP(qmsgAsync.mp1);
                        ROUND_DOWN_MOD(ptsScrollPos.x, (SHORT)lByteAlignX);
                        break;
                    case SB_PAGEUP:
                        ptsScrollPos.x -= ptsScrollPage.x; break;
                    case SB_PAGEDOWN:
                        ptsScrollPos.x += ptsScrollPage.x; break;
                    default: break;
                }
                DoHorzScroll();
                break;

            case UM_VSCROLL:
                switch (SHORT2FROMMP(qmsgAsync.mp1)) {
                    case SB_LINEUP:
                        ptsScrollPos.y -= ptsScrollLine.y; break;
                    case SB_LINEDOWN:
                        ptsScrollPos.y += ptsScrollLine.y; break;
                    case SB_SLIDERTRACK:
                    case SB_SLIDERPOSITION:
                        for (fDone = FALSE; !fDone;) {
                            if (WinPeekMsg(habAsync, &qmsgPeek, NULLHANDLE,
                                           UM_VSCROLL, UM_VSCROLL, PM_NOREMOVE))
                                if ((SHORT2FROMMP(qmsgPeek.mp1) == SB_SLIDERTRACK) ||
                                    (SHORT2FROMMP(qmsgPeek.mp1) == SB_SLIDERPOSITION))
                                    WinPeekMsg(habAsync, &qmsgAsync, NULLHANDLE,
                                               UM_VSCROLL, UM_VSCROLL, PM_REMOVE);
                                else fDone = TRUE;
                            else fDone = TRUE;
                        }
                        ptsScrollPos.y = SHORT1FROMMP(qmsgAsync.mp1);
                        ROUND_DOWN_MOD(ptsScrollPos.y, (SHORT)lByteAlignY);
                        break;
                    case SB_PAGEUP:
                        ptsScrollPos.y -= ptsScrollPage.y; break;
                    case SB_PAGEDOWN:
                        ptsScrollPos.y += ptsScrollPage.y; break;
                    default: break;
                }
                DoVertScroll();
                break;

            case UM_SIZING:
                CalcBounds();
                CalcTransform(hwndClient);
                break;

            case UM_ZOOM:
                sZoomArg = (SHORT)qmsgAsync.mp1;
                while (WinPeekMsg(habAsync, &qmsgPeek, NULLHANDLE,
                                  WM_USER, UM_LAST, PM_NOREMOVE)) {
                    if (qmsgPeek.msg == UM_ZOOM)
                        WinPeekMsg(habAsync, &qmsgPeek, NULLHANDLE,
                                   WM_USER, UM_LAST, PM_REMOVE);
                    else break;
                    sZoomArg += (SHORT)qmsgPeek.mp1;
                }
                if (WinPeekMsg(habAsync, &qmsgPeek, NULLHANDLE,
                               UM_SIZING, UM_LOAD, PM_NOREMOVE))
                    fDrawOn = FALSE;
                else
                    fDrawOn = TRUE;
                Zoom(sZoomArg);
                break;

            case UM_LEFTDOWN:
                if (!fButtonDownAsync) {
                    fButtonDownAsync = TRUE;
                    LeftDown(qmsgAsync.mp1);
                }
                break;

            case UM_MOUSEMOVE:
                if (!fButtonDownAsync) break;
                for (fDone = FALSE; !fDone;) {
                    if (WinPeekMsg(habAsync, &qmsgPeek, NULLHANDLE,
                                   UM_MOUSEMOVE, UM_LEFTUP, PM_NOREMOVE))
                        if (qmsgPeek.msg == UM_MOUSEMOVE)
                            WinPeekMsg(habAsync, &qmsgAsync, NULLHANDLE,
                                       UM_MOUSEMOVE, UM_MOUSEMOVE, PM_REMOVE);
                        else fDone = TRUE;
                    else fDone = TRUE;
                }
                MouseMove(qmsgAsync.mp1);
                break;

            case UM_LEFTUP:
                if (fButtonDownAsync) {
                    LeftUp();
                    fButtonDownAsync = FALSE;
                }
                break;

            case UM_DIE:
                WinDestroyMsgQueue(hmqAsync);
                WinTerminate(habAsync);
                DosExit(EXIT_THREAD, 0);
                break;

            default:
                break;
        }
    }
}

VOID
CalcSize(MPARAM mp1, MPARAM mp2)
{
    ptsScrollMax.y     = SHORT2FROMMP(mp2);
    ptsHalfScrollMax.y = ptsScrollMax.y >> 1;
    ptsScrollPage.x    = ptsScrollMax.x >> 3;
    ROUND_DOWN_MOD(ptsScrollPage.x, (SHORT)lByteAlignX);
    ptsScrollLine.x    = ptsScrollMax.x >> 5;
    ROUND_DOWN_MOD(ptsScrollLine.x, (SHORT)lByteAlignX);
    ptsScrollPos.y = (SHORT)(((LONG)ptsScrollPos.y    * (LONG)SHORT2FROMMP(mp2))
                              / (LONG)SHORT2FROMMP(mp1));
    ptsOldScrollPos.y = (SHORT)(((LONG)ptsOldScrollPos.y * (LONG)SHORT2FROMMP(mp2))
                                 / (LONG)SHORT2FROMMP(mp1));
    WinSendMsg(hwndVertScroll, SBM_SETSCROLLBAR,
               MPFROMSHORT(ptsScrollPos.y),
               MPFROM2SHORT(1, ptsScrollMax.y));

    ptsScrollMax.x     = SHORT1FROMMP(mp2);
    ptsHalfScrollMax.x = ptsScrollMax.x >> 1;
    ptsScrollPage.y    = ptsScrollMax.y >> 3;
    ROUND_DOWN_MOD(ptsScrollPage.y, (SHORT)lByteAlignY);
    ptsScrollLine.y    = ptsScrollMax.y >> 5;
    ROUND_DOWN_MOD(ptsScrollLine.y, (SHORT)lByteAlignY);
    ptsScrollPos.x = (SHORT)(((LONG)ptsScrollPos.x    * (LONG)SHORT1FROMMP(mp2))
                              / (LONG)SHORT1FROMMP(mp1));
    ptsOldScrollPos.x = (SHORT)(((LONG)ptsOldScrollPos.x * (LONG)SHORT1FROMMP(mp2))
                                 / (LONG)SHORT1FROMMP(mp1));
    WinSendMsg(hwndHorzScroll, SBM_SETSCROLLBAR,
               MPFROMSHORT(ptsScrollPos.x),
               MPFROM2SHORT(1, ptsScrollMax.x));
}

VOID
LeftDown(MPARAM mp)
{
    POINTL   aptl[2];
    LONG     lColor;
    POINTL   ptl;
    HRGN     hrgn, hrgnUpdt, hrgnUpdtDrag;
    RECTL    rcl;
    MATRIXLF matlf;
    CHAR     pszMsg[50];
    BOOL     fFirst;
    PSEGLIST psl;

    ptl.x = (LONG)(SHORT)SHORT1FROMMP(mp);
    ptl.y = (LONG)(SHORT)SHORT2FROMMP(mp);

    aptl[0] = aptl[1] = ptl;
    aptl[1].x++;
    aptl[1].y++;
    GpiBitBlt(hpsBitmapSave, NULLHANDLE, 2L, aptl, ROP_ONE, BBO_IGNORE);
    lColor = GpiQueryPel(hpsBitmapSave, &ptl);

    for (pslPicked = pslTail; pslPicked != NULL;
         pslPicked = pslPicked->pslPrev) {
        rcl = pslPicked->rclCurrent;
        GpiConvert(hpsClient, CVTC_MODEL, CVTC_DEVICE, 2L, (PPOINTL)&rcl);
        rcl.xRight++;
        rcl.yTop++;
        if (WinPtInRect(habAsync, &rcl, &ptl)) {
            DrawPiece(hpsBitmapSave, pslPicked, FALSE);
            if (GpiQueryPel(hpsBitmapSave, &ptl) != lColor)
                break;
        }
    }

    if (pslPicked)
        lPickedSeg = pslPicked->lSegId;
    else {
        fButtonDownAsync = FALSE;
        return;
    }
    if ((lPickedSeg < 1) || (lPickedSeg > lLastSegId)) {
        sprintf(pszMsg, "Segment id out of range: %lx", lPickedSeg);
        MyMessageBox(hwndClient, pszMsg);
        fButtonDownAsync = FALSE;
        return;
    }

    GpiQuerySegmentTransformMatrix(hpsClient, lPickedSeg, 9L, &matlf);
    ptlOffStart.x = matlf.lM31;
    ptlOffStart.y = matlf.lM32;
    ptlMoveStart  = ptl;
    GpiConvert(hpsClient, CVTC_DEVICE, CVTC_MODEL, 1L, &ptlMoveStart);
    ptlMoveStart.x = (ptlMoveStart.x / 50) * 50;
    ptlMoveStart.y = (ptlMoveStart.y / 50) * 50;
    ptlUpdtRef     = ptlMoveStart;
    GpiConvert(hpsClient, CVTC_MODEL, CVTC_DEVICE, 1L, &ptlUpdtRef);

    hrgnUpdt = GpiCreateRegion(hpsClient, 0L, NULL);
    for (psl = pslPicked, fFirst = TRUE;
         (psl != pslPicked) || fFirst;
         psl = psl->pslNextIsland, fFirst = FALSE) {
        rcl = psl->rclCurrent;
        GpiConvert(hpsClient, CVTC_MODEL, CVTC_DEVICE, 2L, (PPOINTL)&rcl);
        rcl.xRight++;  rcl.yTop++;
        rcl.xRight += 2; rcl.yTop += 2;
        rcl.xLeft  -= 4; rcl.yBottom -= 4;
        hrgn = GpiCreateRegion(hpsClient, 1L, &rcl);
        GpiCombineRegion(hpsClient, hrgnUpdt, hrgnUpdt, hrgn, CRGN_OR);
        GpiDestroyRegion(hpsClient, hrgn);
        GpiSetSegmentAttrs(hpsClient, psl->lSegId, ATTR_VISIBLE, ATTR_OFF);
    }

    GpiQueryRegionBox(hpsClient, hrgnUpdt, (PRECTL)aptlUpdt);
    WinMapWindowPoints(hwndClient, HWND_DESKTOP, aptlUpdt, 3);
    ROUND_DOWN_MOD(aptlUpdt[0].x, lByteAlignX);
    ROUND_DOWN_MOD(aptlUpdt[0].y, lByteAlignY);
    ROUND_UP_MOD(  aptlUpdt[1].x, lByteAlignX);
    ROUND_UP_MOD(  aptlUpdt[1].y, lByteAlignY);
    WinMapWindowPoints(HWND_DESKTOP, hwndClient, aptlUpdt, 3);
    hrgnUpdtDrag = GpiCreateRegion(hpsBitmapBuff, 1L, (PRECTL)aptlUpdt);

    aptlUpdt[2] = aptlUpdt[0];
    DoDraw(hpsBitmapBuff, hrgnUpdtDrag, TRUE);
    GpiDestroyRegion(hpsClient, hrgnUpdt);
    GpiDestroyRegion(hpsBitmapBuff, hrgnUpdtDrag);
    GpiBitBlt(hpsBitmapSave, hpsBitmapBuff, 3L, aptlUpdt,
              ROP_SRCCOPY, BBO_IGNORE);

    for (psl = pslPicked, fFirst = TRUE;
         (psl != pslPicked) || fFirst;
         psl = psl->pslNextIsland, fFirst = FALSE) {
        GpiSetSegmentAttrs(hpsClient, psl->lSegId, ATTR_VISIBLE, ATTR_ON);
        DrawPiece(hpsBitmapBuff, psl, TRUE);
    }
    GpiBitBlt(hpsClient, hpsBitmapBuff, 3L, aptlUpdt,
              ROP_SRCCOPY, BBO_IGNORE);
    WinSetCapture(HWND_DESKTOP, hwndClient);
}

VOID
MouseMove(MPARAM mp)
{
    RECTL    rcl;
    POINTL   ptl, ptlModel, ptlDevice;
    POINTL   aptlUpdtRef[3], aptlUpdtNew[3];
    PSEGLIST psl;
    BOOL     fFirst;
    MATRIXLF matlf;

    if (!lPickedSeg || !pslPicked) return;

    ptl.x = (LONG)(SHORT)SHORT1FROMMP(mp);
    ptl.y = (LONG)(SHORT)SHORT2FROMMP(mp);

    WinQueryWindowRect(hwndClient, &rcl);
    if (rcl.xLeft   > ptl.x) ptl.x = rcl.xLeft;
    if (rcl.xRight  <= ptl.x) ptl.x = rcl.xRight;
    if (rcl.yBottom > ptl.y) ptl.y = rcl.yBottom;
    if (rcl.yTop    <= ptl.y) ptl.y = rcl.yTop;

    ptlModel = ptl;
    GpiConvert(hpsClient, CVTC_DEVICE, CVTC_MODEL, 1L, &ptlModel);
    ptlModel.x = 50 * (ptlModel.x / 50);
    ptlModel.y = 50 * (ptlModel.y / 50);
    if ((ptlModel.x == ptlOldMouse.x) && (ptlModel.y == ptlOldMouse.y))
        return;
    ptlOldMouse.x = ptlModel.x;
    ptlOldMouse.y = ptlModel.y;
    ptlDevice = ptlModel;
    GpiConvert(hpsClient, CVTC_MODEL, CVTC_DEVICE, 1L, &ptlDevice);

    GpiBitBlt(hpsBitmapBuff, hpsBitmapSave, 3L, aptlUpdt,
              ROP_SRCCOPY, BBO_IGNORE);
    aptlUpdtRef[0] = aptlUpdt[0];
    aptlUpdtRef[1] = aptlUpdt[1];

    aptlUpdt[0].x += ptlDevice.x - ptlUpdtRef.x;
    aptlUpdt[0].y += ptlDevice.y - ptlUpdtRef.y;
    aptlUpdt[1].x += ptlDevice.x - ptlUpdtRef.x;
    aptlUpdt[1].y += ptlDevice.y - ptlUpdtRef.y;
    WinMapWindowPoints(hwndClient, HWND_DESKTOP, aptlUpdt, 3);
    ROUND_DOWN_MOD(aptlUpdt[0].x, lByteAlignX);
    ROUND_DOWN_MOD(aptlUpdt[0].y, lByteAlignY);
    ROUND_UP_MOD(  aptlUpdt[1].x, lByteAlignX);
    ROUND_UP_MOD(  aptlUpdt[1].y, lByteAlignY);
    WinMapWindowPoints(HWND_DESKTOP, hwndClient, aptlUpdt, 3);
    aptlUpdt[2] = aptlUpdt[0];
    ptlUpdtRef  = ptlDevice;
    GpiBitBlt(hpsBitmapSave, hpsBitmapBuff, 3L, aptlUpdt,
              ROP_SRCCOPY, BBO_IGNORE);

    GpiQuerySegmentTransformMatrix(hpsClient, lPickedSeg, 9L, &matlf);
    for (psl = pslPicked, fFirst = TRUE;
         (psl != pslPicked) || fFirst;
         psl = psl->pslNextIsland, fFirst = FALSE) {
        matlf.lM31 = ptlOffStart.x + ptlModel.x - ptlMoveStart.x;
        matlf.lM32 = ptlOffStart.y + ptlModel.y - ptlMoveStart.y;
        GpiSetSegmentTransformMatrix(hpsClient, psl->lSegId, 9L,
                                     &matlf, TRANSFORM_REPLACE);
        psl->ptlModelXlate.x = matlf.lM31;
        psl->ptlModelXlate.y = matlf.lM32;
        DrawPiece(hpsBitmapBuff, psl, TRUE);
    }

    WinUnionRect(habMain, (PRECTL)aptlUpdtNew,
                 (PRECTL)aptlUpdt, (PRECTL)aptlUpdtRef);
    WinMapWindowPoints(hwndClient, HWND_DESKTOP, aptlUpdtNew, 2);
    ROUND_DOWN_MOD(aptlUpdtNew[0].x, lByteAlignX);
    ROUND_DOWN_MOD(aptlUpdtNew[0].y, lByteAlignY);
    ROUND_UP_MOD(  aptlUpdtNew[1].x, lByteAlignX);
    ROUND_UP_MOD(  aptlUpdtNew[1].y, lByteAlignY);
    WinMapWindowPoints(HWND_DESKTOP, hwndClient, aptlUpdtNew, 2);
    aptlUpdtNew[2] = aptlUpdtNew[0];
    GpiBitBlt(hpsClient, hpsBitmapBuff, 3L, aptlUpdtNew,
              ROP_SRCCOPY, BBO_IGNORE);
}

VOID
LeftUp(VOID)
{
    PSEGLIST psl, pslTemp;
    POINTL   ptlShift;
    BOOL     fFirst;
    MATRIXLF matlf;
    LONG     l;

    if (!lPickedSeg || !pslPicked) return;

    for (psl = pslPicked, fFirst = TRUE;
         (psl != pslPicked) || fFirst;
         psl = psl->pslNextIsland, fFirst = FALSE) {
        SetRect(psl);
        SegListUpdate(MAKE_TAIL_SEG, psl);
        psl->fIslandMark = TRUE;
    }

    GpiQuerySegmentTransformMatrix(hpsClient, lPickedSeg, 9L, &matlf);
    ptlShift.x = matlf.lM31;
    ptlShift.y = matlf.lM32;

    for (psl = pslHead; psl != NULL; psl = psl->pslNext)
        if (!psl->fIslandMark)
            for (l = 0; l < 8; l++)
                if (pslPicked->lAdjacent[l] == psl->lSegId) {
                    GpiQuerySegmentTransformMatrix(hpsClient,
                                                  psl->lSegId, 9L, &matlf);
                    if ((ptlShift.x == matlf.lM31) &&
                        (ptlShift.y == matlf.lM32)) {
                        DosBeep(600,  100);
                        DosBeep(1200, 50);
                        MarkIsland(psl, TRUE);
                        pslTemp                  = psl->pslNextIsland;
                        psl->pslNextIsland       = pslPicked->pslNextIsland;
                        pslPicked->pslNextIsland = pslTemp;
                    }
                }

    MarkIsland(pslPicked, FALSE);
    pslPicked  = NULL;
    lPickedSeg = 0L;
    WinSetCapture(HWND_DESKTOP, NULLHANDLE);
}

VOID
MarkIsland(PSEGLIST pslMark, BOOL fMark)
{
    PSEGLIST psl;
    BOOL     fFirst;

    for (psl = pslMark, fFirst = TRUE;
         (psl != pslMark) || fFirst;
         psl = psl->pslNextIsland, fFirst = FALSE)
        psl->fIslandMark = fMark;
}

VOID
DoHorzScroll(VOID)
{
    POINTL   aptlClient[3];
    HRGN     hrgn;
    MATRIXLF matlf;

    if (ptsScrollPos.x > ptsScrollMax.x) ptsScrollPos.x = ptsScrollMax.x;
    if (ptsScrollPos.x < 0)              ptsScrollPos.x = 0;

    if (ptsOldScrollPos.x != ptsScrollPos.x)
        WinSendMsg(hwndHorzScroll, SBM_SETPOS,
                   MPFROM2SHORT(ptsScrollPos.x, 0),
                   MPFROMLONG(0L));

    hrgn = GpiCreateRegion(hpsClient, 0L, NULL);
    WinQueryWindowRect(hwndClient, (PRECTL)aptlClient);
    if (abs(ptsScrollPos.x - ptsOldScrollPos.x) <= ptsScrollMax.x)
        WinScrollWindow(hwndClient,
                        ptsOldScrollPos.x - ptsScrollPos.x, 0,
                        NULL, NULL, hrgn, NULL, 0);
    else
        GpiSetRegion(hpsClient, hrgn, 1L, (PRECTL)aptlClient);

    GpiQueryDefaultViewMatrix(hpsClient, 9L, &matlf);
    matlf.lM31 -= ptsScrollPos.x - ptsOldScrollPos.x;
    GpiSetDefaultViewMatrix(hpsClient, 9L, &matlf, TRANSFORM_REPLACE);

    DoDraw(hpsClient, hrgn, TRUE);
    ptsOldScrollPos.x = ptsScrollPos.x;
    GpiDestroyRegion(hpsClient, hrgn);

    aptlClient[2] = aptlClient[0];
    GpiBitBlt(hpsBitmapBuff, hpsClient, 3L, aptlClient,
              ROP_SRCCOPY, BBO_IGNORE);
}

VOID
DoVertScroll(VOID)
{
    POINTL   aptlClient[3];
    HRGN     hrgn;
    MATRIXLF matlf;

    if (ptsScrollPos.y > ptsScrollMax.y) ptsScrollPos.y = ptsScrollMax.y;
    if (ptsScrollPos.y < 0)              ptsScrollPos.y = 0;

    if (ptsOldScrollPos.y != ptsScrollPos.y)
        WinSendMsg(hwndVertScroll, SBM_SETPOS,
                   MPFROM2SHORT(ptsScrollPos.y, 0),
                   MPFROMLONG(0L));

    hrgn = GpiCreateRegion(hpsClient, 0L, NULL);
    WinQueryWindowRect(hwndClient, (PRECTL)aptlClient);
    if (abs(ptsScrollPos.y - ptsOldScrollPos.y) <= ptsScrollMax.y)
        WinScrollWindow(hwndClient, 0,
                        ptsScrollPos.y - ptsOldScrollPos.y,
                        NULL, NULL, hrgn, NULL, 0);
    else
        GpiSetRegion(hpsClient, hrgn, 1L, (PRECTL)aptlClient);

    GpiQueryDefaultViewMatrix(hpsClient, 9L, &matlf);
    matlf.lM32 += ptsScrollPos.y - ptsOldScrollPos.y;
    GpiSetDefaultViewMatrix(hpsClient, 9L, &matlf, TRANSFORM_REPLACE);

    DoDraw(hpsClient, hrgn, TRUE);
    ptsOldScrollPos.y = ptsScrollPos.y;
    GpiDestroyRegion(hpsClient, hrgn);

    aptlClient[2] = aptlClient[0];
    GpiBitBlt(hpsBitmapBuff, hpsClient, 3L, aptlClient,
              ROP_SRCCOPY, BBO_IGNORE);
}

VOID
Redraw(VOID)
{
    RECTL   rclInvalid;
    HRGN    hrgnUpdt;
    POINTL  aptlUpdtNew[3];

    WinQueryWindowRect(hwndClient, &rclInvalid);
    hrgnUpdt = GpiCreateRegion(hpsBitmapBuff, 1L, &rclInvalid);
    DoDraw(hpsBitmapBuff, hrgnUpdt, TRUE);
    GpiDestroyRegion(hpsBitmapBuff, hrgnUpdt);

    aptlUpdtNew[0].x = rclInvalid.xLeft;
    aptlUpdtNew[0].y = rclInvalid.yBottom;
    aptlUpdtNew[1].x = rclInvalid.xRight;
    aptlUpdtNew[1].y = rclInvalid.yTop;
    ROUND_DOWN_MOD(aptlUpdtNew[0].x, lByteAlignX);
    ROUND_DOWN_MOD(aptlUpdtNew[0].y, lByteAlignY);
    ROUND_UP_MOD(  aptlUpdtNew[1].x, lByteAlignX);
    ROUND_UP_MOD(  aptlUpdtNew[1].y, lByteAlignY);
    aptlUpdtNew[2] = aptlUpdtNew[0];
    GpiBitBlt(hpsClient, hpsBitmapBuff, 3L, aptlUpdtNew,
              ROP_SRCCOPY, BBO_IGNORE);
}

VOID
ToggleMenuItem(USHORT usMenuMajor, USHORT usMenuMinor, PBOOL pfFlag)
{
    MENUITEM mi;

    WinSendMsg(WinWindowFromID(hwndFrame, FID_MENU),
               MM_QUERYITEM,
               MPFROM2SHORT(usMenuMajor, FALSE),
               MPFROMP((PMENUITEM)&mi));

    if (*pfFlag) {
        *pfFlag = FALSE;
        WinSendMsg(mi.hwndSubMenu, MM_SETITEMATTR,
                   MPFROM2SHORT(usMenuMinor, TRUE),
                   MPFROM2SHORT(MIA_CHECKED, (USHORT)~MIA_CHECKED));
    } else {
        *pfFlag = TRUE;
        WinSendMsg(mi.hwndSubMenu, MM_SETITEMATTR,
                   MPFROM2SHORT(usMenuMinor, TRUE),
                   MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
    }
}

VOID
Zoom(SHORT sInOrOut)
{
    LONG lScaleOld = lScale;

    lScale += sInOrOut;
    if (lScale > 0)        lScale = 0;
    if (lScale < -ZOOM_MAX) lScale = -ZOOM_MAX;

    if (lScale != lScaleOld) {
        ZoomMenuItems();
        CalcBounds();
        CalcTransform(hwndClient);
        Redraw();
    }
}

VOID
ZoomMenuItems(VOID)
{
    MENUITEM mi;
    HWND     hwndMenu, hwndOptions;

    hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);
    if (!hwndMenu) return;

    WinSendMsg(hwndMenu, MM_QUERYITEM,
               MPFROM2SHORT(IDM_SUBMENU_OPTIONS, FALSE),
               MPFROMP((PMENUITEM)&mi));
    hwndOptions = mi.hwndSubMenu;

    if (lScale >= 0) {
        WinSendMsg(hwndOptions, MM_SETITEMATTR,
                   MPFROM2SHORT(IDM_ZOOMIN, TRUE),
                   MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
        WinSendMsg(hwndOptions, MM_SETITEMATTR,
                   MPFROM2SHORT(IDM_ZOOMOUT, TRUE),
                   MPFROM2SHORT(MIA_DISABLED, (USHORT)~MIA_DISABLED));
    } else if (lScale <= -ZOOM_MAX) {
        WinSendMsg(hwndOptions, MM_SETITEMATTR,
                   MPFROM2SHORT(IDM_ZOOMOUT, TRUE),
                   MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
        WinSendMsg(hwndOptions, MM_SETITEMATTR,
                   MPFROM2SHORT(IDM_ZOOMIN, TRUE),
                   MPFROM2SHORT(MIA_DISABLED, (USHORT)~MIA_DISABLED));
    } else {
        WinSendMsg(hwndOptions, MM_SETITEMATTR,
                   MPFROM2SHORT(IDM_ZOOMOUT, TRUE),
                   MPFROM2SHORT(MIA_DISABLED, (USHORT)~MIA_DISABLED));
        WinSendMsg(hwndOptions, MM_SETITEMATTR,
                   MPFROM2SHORT(IDM_ZOOMIN, TRUE),
                   MPFROM2SHORT(MIA_DISABLED, (USHORT)~MIA_DISABLED));
    }
}

VOID
SetRect(PSEGLIST psl)
{
    MATRIXLF matlf;

    GpiQuerySegmentTransformMatrix(hpsClient, psl->lSegId, 9L, &matlf);
    psl->rclCurrent = psl->rclBitBlt;
    psl->rclCurrent.xLeft   += matlf.lM31;
    psl->rclCurrent.yBottom += matlf.lM32;
    psl->rclCurrent.xRight  += matlf.lM31;
    psl->rclCurrent.yTop    += matlf.lM32;
}

VOID
SetDVTransform(FIXED fx11, FIXED fx12, FIXED fx21, FIXED fx22,
               LONG l31, LONG l32, LONG lType)
{
    MATRIXLF matlf;

    matlf.fxM11 = fx11;  matlf.fxM12 = fx12;  matlf.lM13 = 0L;
    matlf.fxM21 = fx21;  matlf.fxM22 = fx22;  matlf.lM23 = 0L;
    matlf.lM31  = l31;   matlf.lM32  = l32;   matlf.lM33 = 1L;
    GpiSetDefaultViewMatrix(hpsClient, 9L, &matlf, lType);
}

VOID
CalcBounds(VOID)
{
    PSEGLIST psl;
    RECTL    rcl;

    if (!pslHead) return;
    rclBounds = pslHead->rclCurrent;
    for (psl = pslHead->pslNext; psl != NULL; psl = psl->pslNext) {
        rcl = psl->rclCurrent;
        if (rcl.xLeft   < rclBounds.xLeft)   rclBounds.xLeft   = rcl.xLeft;
        if (rcl.xRight  > rclBounds.xRight)  rclBounds.xRight  = rcl.xRight;
        if (rcl.yTop    > rclBounds.yTop)    rclBounds.yTop    = rcl.yTop;
        if (rcl.yBottom < rclBounds.yBottom) rclBounds.yBottom = rcl.yBottom;
    }
}

VOID
CalcTransform(HWND hwnd)
{
    RECTL    rclClient;
    POINTL   ptlCenter, ptlTrans, ptlScale, aptl[4], ptlOrg, aptlSides[12];
    PSEGLIST psl;
    LONG     l;

    ptlCenter.x = (rclBounds.xLeft   + rclBounds.xRight) / 2;
    ptlCenter.y = (rclBounds.yBottom + rclBounds.yTop)   / 2;

    SetDVTransform((FIXED)UNITY, (FIXED)0, (FIXED)0, (FIXED)UNITY,
                   -ptlCenter.x, -ptlCenter.y, TRANSFORM_REPLACE);

    ptlScale.x = UNITY * bmpBitmapFile.cx / (ptlTopRight.x - ptlBotLeft.x);
    ptlScale.y = UNITY * bmpBitmapFile.cy / (ptlTopRight.y - ptlBotLeft.y);
    ptlScale.x += ptlScale.x * lScale / (ZOOM_MAX + 1);
    ptlScale.y += ptlScale.y * lScale / (ZOOM_MAX + 1);

    SetDVTransform((FIXED)ptlScale.x, (FIXED)0, (FIXED)0, (FIXED)ptlScale.y,
                   0L, 0L, TRANSFORM_ADD);

    WinQueryWindowRect(hwnd, &rclClient);
    ptlTrans.x = (rclClient.xRight - rclClient.xLeft)   / 2;
    ptlTrans.y = (rclClient.yTop   - rclClient.yBottom)  / 2;
    ptlTrans.x += ptsScrollPos.x - ptsHalfScrollMax.x;
    ptlTrans.y += ptsScrollPos.y - ptsHalfScrollMax.y;

    SetDVTransform((FIXED)UNITY, (FIXED)0, (FIXED)0, (FIXED)UNITY,
                   ptlTrans.x, ptlTrans.y, TRANSFORM_ADD);

    ptlOffset = ptlBotLeft;
    GpiConvert(hpsClient, CVTC_WORLD, CVTC_DEVICE, 1L, &ptlOffset);

    if ((ptlScale.x != ptlScaleRef.x) || (ptlScale.y != ptlScaleRef.y)) {
        ptlScaleRef = ptlScale;

        aptl[0] = ptlBotLeft;
        aptl[1] = ptlTopRight;
        GpiConvert(hpsClient, CVTC_WORLD, CVTC_DEVICE, 2L, aptl);
        aptl[0].x -= ptlOffset.x;
        aptl[0].y -= ptlOffset.y;
        aptl[1].x -= ptlOffset.x - 1;
        aptl[1].y -= ptlOffset.y - 1;
        aptl[2].x  = 0L;
        aptl[2].y  = 0L;
        aptl[3].x  = bmpBitmapFile.cx + 1;
        aptl[3].y  = bmpBitmapFile.cy + 1;
        GpiSetBitmap(hpsBitmapSize, hbmBitmapSize);
        GpiBitBlt(hpsBitmapSize, hpsBitmapFile, 4L, aptl,
                  ROP_SRCCOPY, BBO_IGNORE);

        for (psl = pslHead; psl != NULL; psl = psl->pslNext) {
            if (fTerminate) break;

            aptl[0].x = psl->rclBitBlt.xLeft;
            aptl[0].y = psl->rclBitBlt.yBottom;
            aptl[1].x = psl->rclBitBlt.xRight;
            aptl[1].y = psl->rclBitBlt.yTop;
            aptl[2]   = aptl[0];
            aptl[3]   = aptl[1];
            GpiConvert(hpsClient, CVTC_WORLD, CVTC_DEVICE, 2L, &aptl[2]);
            ptlOrg    = aptl[2];
            aptl[3].x -= ptlOrg.x - 1;
            aptl[3].y -= ptlOrg.y - 1;
            aptl[2].x  = 0L;
            aptl[2].y  = 0L;
            psl->aptlBitBlt[0] = aptl[0];
            psl->aptlBitBlt[1] = aptl[1];
            psl->aptlBitBlt[2] = aptl[2];
            psl->aptlBitBlt[3] = aptl[3];

            for (l = 0; l < 12; l++) aptlSides[l] = psl->aptlSides[l];
            GpiConvert(hpsClient, CVTC_WORLD, CVTC_DEVICE, 12L, aptlSides);
            for (l = 0; l < 12; l++) {
                aptlSides[l].x -= ptlOrg.x;
                aptlSides[l].y -= ptlOrg.y;
            }

            GpiSetBitmap(psl->hpsHole, psl->hbmHole);
            GpiSetClipPath(psl->hpsHole, 0L, SCP_RESET);
            GpiBitBlt(psl->hpsHole, NULLHANDLE, 2L, &psl->aptlBitBlt[2],
                      ROP_ONE, BBO_IGNORE);
            GpiBeginPath(psl->hpsHole, 1L);
            GpiMove(psl->hpsHole, &aptlSides[11]);
            GpiPolySpline(psl->hpsHole, 12L, aptlSides);
            GpiEndPath(psl->hpsHole);
            GpiSetClipPath(psl->hpsHole, 1L, SCP_AND);
            GpiBitBlt(psl->hpsHole, NULLHANDLE, 2L, &psl->aptlBitBlt[2],
                      ROP_ZERO, BBO_IGNORE);
            GpiSetClipPath(psl->hpsHole, 0L, SCP_RESET);

            GpiSetBitmap(psl->hpsFill, psl->hbmFill);
            GpiSetClipPath(psl->hpsFill, 0L, SCP_RESET);
            GpiBitBlt(psl->hpsFill, NULLHANDLE, 2L, &psl->aptlBitBlt[2],
                      ROP_ZERO, BBO_IGNORE);
            GpiBeginPath(psl->hpsFill, 1L);
            GpiMove(psl->hpsFill, &aptlSides[11]);
            GpiPolySpline(psl->hpsFill, 12L, aptlSides);
            GpiEndPath(psl->hpsFill);
            GpiSetClipPath(psl->hpsFill, 1L, SCP_AND);
            aptl[0] = psl->aptlBitBlt[2];
            aptl[1] = psl->aptlBitBlt[3];
            aptl[2].x = ptlOrg.x - ptlOffset.x;
            aptl[2].y = ptlOrg.y - ptlOffset.y;
            GpiBitBlt(psl->hpsFill, hpsBitmapSize, 3L, aptl,
                      ROP_SRCCOPY, BBO_IGNORE);
            GpiSetClipPath(psl->hpsFill, 0L, SCP_RESET);

            GpiSetColor(psl->hpsFill, CLR_RED);
            GpiMove(psl->hpsFill, &aptlSides[11]);
            GpiPolySpline(psl->hpsFill, 12L, aptlSides);
        }
    }
}

VOID
DrawPiece(HPS hps, PSEGLIST psl, BOOL fFill)
{
    POINTL aptl[4];

    if (GpiQuerySegmentAttrs(hpsClient, psl->lSegId, ATTR_VISIBLE) == ATTR_ON) {
        aptl[2].x = aptl[2].y = 0L;
        aptl[3]   = psl->aptlBitBlt[3];
        aptl[0].x = psl->rclBitBlt.xLeft   + psl->ptlModelXlate.x;
        aptl[0].y = psl->rclBitBlt.yBottom  + psl->ptlModelXlate.y;
        GpiConvert(hpsClient, CVTC_MODEL, CVTC_DEVICE, 1L, aptl);
        aptl[1].x = aptl[0].x + aptl[3].x;
        aptl[1].y = aptl[0].y + aptl[3].y;
        GpiBitBlt(hps, psl->hpsHole, 4L, aptl, ROP_SRCAND,   BBO_IGNORE);
        if (fFill)
            GpiBitBlt(hps, psl->hpsFill, 4L, aptl, ROP_SRCPAINT, BBO_IGNORE);
    }
}

BOOL
DoDraw(HPS hps, HRGN hrgn, BOOL fPaint)
{
    HRGN    hrgnOld;
    RECTL   rcl, rclRegion, rclDst, rclClient;
    PSEGLIST psl;

    if (fPaint) {
        GpiSetColor(hps, CLR_BACKGROUND);
        GpiPaintRegion(hps, hrgn);
    }
    GpiQueryRegionBox(hps, hrgn, &rclRegion);
    WinQueryWindowRect(hwndClient, &rclClient);
    if (!WinIntersectRect(habAsync, &rclRegion, &rclRegion, &rclClient))
        return FALSE;

    GpiSetClipRegion(hps, hrgn, &hrgnOld);
    for (psl = pslHead; psl != NULL; psl = psl->pslNext) {
        rcl = psl->rclCurrent;
        GpiConvert(hpsClient, CVTC_MODEL, CVTC_DEVICE, 2L, (PPOINTL)&rcl);
        rcl.xRight++;
        rcl.yTop++;
        if (WinIntersectRect(habAsync, &rclDst, &rcl, &rclRegion))
            if (fDrawOn)
                DrawPiece(hps, psl, TRUE);
            else
                break;
    }
    GpiSetClipRegion(hps, NULLHANDLE, &hrgnOld);
    if (fDrawOn)
        GpiSetRegion(hpsClient, hrgnInvalid, 0L, NULL);

    return TRUE;
}

PSEGLIST
SegListGet(LONG lSeg)
{
    PSEGLIST psl;
    for (psl = pslHead; psl != NULL; psl = psl->pslNext)
        if (psl->lSegId == lSeg) return psl;
    return NULL;
}

BOOL
SegListCheck(INT iLoc)
{
    PSEGLIST psl;
    CHAR     pszMsg[60];

    for (psl = pslHead; psl != NULL; psl = psl->pslNext)
        if ((psl->lSegId < 1) || (psl->lSegId > lLastSegId)) {
            sprintf(pszMsg, "Bad head segment list, location %d", iLoc);
            MyMessageBox(hwndClient, pszMsg);
            return FALSE;
        }
    for (psl = pslTail; psl != NULL; psl = psl->pslPrev)
        if ((psl->lSegId < 1) || (psl->lSegId > lLastSegId)) {
            sprintf(pszMsg, "Bad tail segment list, location %d", iLoc);
            MyMessageBox(hwndClient, pszMsg);
            return FALSE;
        }
    return TRUE;
}

PSEGLIST
SegListUpdate(USHORT usOperation, PSEGLIST pslUpdate)
{
    PSEGLIST psl;
    PVOID    pv;

    switch (usOperation) {
        case ADD_HEAD_SEG:
            if (DosAllocMem(&pv, sizeof(SEGLIST),
                            PAG_READ | PAG_WRITE | PAG_COMMIT) != 0)
                return NULL;
            psl = (PSEGLIST)pv;
            *psl = *pslUpdate;
            if (pslHead == NULL) {
                psl->pslPrev = NULL;
                psl->pslNext = NULL;
                pslHead = pslTail = psl;
            } else {
                psl->pslPrev  = NULL;
                psl->pslNext  = pslHead;
                pslHead->pslPrev = psl;
                pslHead = psl;
            }
            return pslHead;

        case ADD_TAIL_SEG:
            if (DosAllocMem(&pv, sizeof(SEGLIST),
                            PAG_READ | PAG_WRITE | PAG_COMMIT) != 0)
                return NULL;
            psl = (PSEGLIST)pv;
            *psl = *pslUpdate;
            if (pslTail == NULL) {
                psl->pslPrev = NULL;
                psl->pslNext = NULL;
                pslHead = pslTail = psl;
            } else {
                psl->pslPrev  = pslTail;
                psl->pslNext  = NULL;
                pslTail->pslNext = psl;
                pslTail = psl;
            }
            return pslTail;

        case MAKE_TAIL_SEG:
            if (pslUpdate == pslTail) return pslTail;
            if (pslUpdate == pslHead) {
                pslHead          = pslHead->pslNext;
                pslHead->pslPrev = NULL;
            } else {
                pslUpdate->pslPrev->pslNext = pslUpdate->pslNext;
                pslUpdate->pslNext->pslPrev = pslUpdate->pslPrev;
            }
            pslTail->pslNext    = pslUpdate;
            pslUpdate->pslPrev  = pslTail;
            pslTail             = pslUpdate;
            pslTail->pslNext    = NULL;
            return pslTail;

        case DEL_SEG:
            for (psl = pslHead; psl != NULL; psl = psl->pslNext)
                if (psl->lSegId == pslUpdate->lSegId) {
                    if (psl == pslHead) {
                        pslHead = psl->pslNext;
                        if (pslHead) pslHead->pslPrev = NULL;
                        else         pslTail = NULL;
                    } else if (psl == pslTail) {
                        pslTail = psl->pslPrev;
                        pslTail->pslNext = NULL;
                    } else {
                        psl->pslPrev->pslNext = psl->pslNext;
                        psl->pslNext->pslPrev = psl->pslPrev;
                    }
                    DosFreeMem(psl);
                    return psl;
                }
            return NULL;

        default:
            return NULL;
    }
}

BOOL
DumpPicture(VOID)
{
    while (pslHead != NULL) {
        GpiSetBitmap(pslHead->hpsFill, NULLHANDLE);
        GpiDeleteBitmap(pslHead->hbmFill);
        GpiDestroyPS(pslHead->hpsFill);
        DevCloseDC(pslHead->hdcFill);

        GpiSetBitmap(pslHead->hpsHole, NULLHANDLE);
        GpiDeleteBitmap(pslHead->hbmHole);
        GpiDestroyPS(pslHead->hpsHole);
        DevCloseDC(pslHead->hdcHole);

        SegListUpdate(DEL_SEG, pslHead);
    }
    if (lLastSegId > 0)
        GpiDeleteSegments(hpsClient, 1L, lLastSegId);

    if (hbmBitmapFile) { GpiSetBitmap(hpsBitmapFile, NULLHANDLE); GpiDeleteBitmap(hbmBitmapFile); hbmBitmapFile = NULLHANDLE; }
    if (hbmBitmapSize) { GpiSetBitmap(hpsBitmapSize, NULLHANDLE); GpiDeleteBitmap(hbmBitmapSize); hbmBitmapSize = NULLHANDLE; }
    if (hbmBitmapBuff) { GpiSetBitmap(hpsBitmapBuff, NULLHANDLE); GpiDeleteBitmap(hbmBitmapBuff); hbmBitmapBuff = NULLHANDLE; }
    if (hbmBitmapSave) { GpiSetBitmap(hpsBitmapSave, NULLHANDLE); GpiDeleteBitmap(hbmBitmapSave); hbmBitmapSave = NULLHANDLE; }
    return TRUE;
}

BOOL
CreatePicture(VOID)
{
    POINTL   ptl, aptlSides[12], aptlControl[12];
    SEGLIST  sl;
    PSEGLIST psl;
    LONG     l, lMinor, lNeighbor, alFuzz[36][4];
    SIZEL    sizl;
    BITMAPINFOHEADER bmp;
    DATETIME date;

    DosGetDateTime(&date);
    srand((USHORT)date.hundredths);
    for (l = 0; l < 36; l++)
        for (lMinor = 0; lMinor < 4; lMinor++)
            alFuzz[l][lMinor] = 50 * (rand() % 10);

    SetDVTransform((FIXED)UNITY, (FIXED)0, (FIXED)0, (FIXED)UNITY,
                   0L, 0L, TRANSFORM_REPLACE);

    GpiSetDrawingMode(hpsClient, DM_RETAIN);
    GpiSetInitialSegmentAttrs(hpsClient, ATTR_CHAINED,    ATTR_OFF);
    GpiSetInitialSegmentAttrs(hpsClient, ATTR_FASTCHAIN,  ATTR_OFF);
    GpiSetInitialSegmentAttrs(hpsClient, ATTR_DETECTABLE, ATTR_OFF);

    lLastSegId = 0;
    for (ptl.x = ptlBotLeft.x; ptl.x < ptlTopRight.x; ptl.x += 500) {
        if (fTerminate) break;
        for (ptl.y = ptlBotLeft.y; ptl.y < ptlTopRight.y; ptl.y += 500) {
            if (fTerminate) break;
            lLastSegId++;

            aptlControl[ 0].x = 250L;  aptlControl[ 0].y =  500L;
            aptlControl[ 1].x = 250L;  aptlControl[ 1].y = -500L;
            aptlControl[ 2].x = 500L;  aptlControl[ 2].y =    0L;
            aptlControl[ 3].x =   0L;  aptlControl[ 3].y =  250L;
            aptlControl[ 4].x =1000L;  aptlControl[ 4].y =  250L;
            aptlControl[ 5].x = 500L;  aptlControl[ 5].y =  500L;
            aptlControl[ 6].x = 250L;  aptlControl[ 6].y =    0L;
            aptlControl[ 7].x = 250L;  aptlControl[ 7].y = 1000L;
            aptlControl[ 8].x =   0L;  aptlControl[ 8].y =  500L;
            aptlControl[ 9].x = 500L;  aptlControl[ 9].y =  250L;
            aptlControl[10].x =-500L;  aptlControl[10].y =  250L;
            aptlControl[11].x =   0L;  aptlControl[11].y =    0L;

            if (ptl.y == ptlBotLeft.y)
                aptlControl[0].y = aptlControl[1].y = 0L;
            if ((ptl.x + 500) == ptlTopRight.x)
                aptlControl[3].x = aptlControl[4].x = 500L;
            if ((ptl.y + 500) == ptlTopRight.y)
                aptlControl[6].y = aptlControl[7].y = 500L;
            if (ptl.x == ptlBotLeft.x)
                aptlControl[9].x = aptlControl[10].x = 0L;

            sl.lAdjacent[0] = lLastSegId - 7;
            sl.lAdjacent[1] = lLastSegId - 6;
            sl.lAdjacent[2] = lLastSegId - 5;
            sl.lAdjacent[3] = lLastSegId - 1;
            sl.lAdjacent[4] = lLastSegId + 1;
            sl.lAdjacent[5] = lLastSegId + 5;
            sl.lAdjacent[6] = lLastSegId + 6;
            sl.lAdjacent[7] = lLastSegId + 7;
            if (ptl.x == ptlBotLeft.x)
                sl.lAdjacent[0] = sl.lAdjacent[1] = sl.lAdjacent[2] = 0;
            if (ptl.y == ptlBotLeft.y)
                sl.lAdjacent[0] = sl.lAdjacent[3] = sl.lAdjacent[5] = 0;
            if ((ptl.x + 500) == ptlTopRight.x)
                sl.lAdjacent[5] = sl.lAdjacent[6] = sl.lAdjacent[7] = 0;
            if ((ptl.y + 500) == ptlTopRight.y)
                sl.lAdjacent[2] = sl.lAdjacent[4] = sl.lAdjacent[7] = 0;

            if (sl.lAdjacent[3]) {
                aptlControl[0].y -= alFuzz[lLastSegId-1][0];
                aptlControl[1].y += alFuzz[lLastSegId-1][1];
            }
            if (sl.lAdjacent[1]) {
                aptlControl[ 9].x -= alFuzz[lLastSegId-1][2];
                aptlControl[10].x += alFuzz[lLastSegId-1][3];
            }
            if ((lNeighbor = sl.lAdjacent[4]) != 0) {
                aptlControl[7].y -= alFuzz[lNeighbor-1][0];
                aptlControl[6].y += alFuzz[lNeighbor-1][1];
            }
            if ((lNeighbor = sl.lAdjacent[6]) != 0) {
                aptlControl[4].x -= alFuzz[lNeighbor-1][2];
                aptlControl[3].x += alFuzz[lNeighbor-1][3];
            }

            for (l = 0; l < 12; l++) {
                aptlSides[l].x  = ptl.x + aptlControl[l].x;
                aptlSides[l].y  = ptl.y + aptlControl[l].y;
                sl.aptlSides[l] = aptlSides[l];
            }

            sl.rclBitBlt.xLeft   = ptl.x - 250;
            sl.rclBitBlt.yBottom = ptl.y - 250;
            sl.rclBitBlt.xRight  = ptl.x + 750;
            sl.rclBitBlt.yTop    = ptl.y + 750;
            if (ptl.x == ptlBotLeft.x)             sl.rclBitBlt.xLeft   += 250;
            if (ptl.y == ptlBotLeft.y)             sl.rclBitBlt.yBottom += 250;
            if ((ptl.x+500) == ptlTopRight.x)      sl.rclBitBlt.xRight  -= 250;
            if ((ptl.y+500) == ptlTopRight.y)      sl.rclBitBlt.yTop    -= 250;

            sl.ptlLocation = ptl;

            GpiOpenSegment(hpsClient, lLastSegId);
            GpiSetTag(hpsClient, lLastSegId);
            GpiBeginPath(hpsClient, 1L);
            GpiMove(hpsClient, &ptl);
            GpiPolySpline(hpsClient, 12L, aptlSides);
            GpiEndPath(hpsClient);
            GpiSetColor(hpsClient, CLR_BLACK);
            GpiFillPath(hpsClient, 1L, 0L);
            GpiCloseSegment(hpsClient);

            sizl.cx = 2 + ((LONG)bmpBitmapFile.cx
                           * (sl.rclBitBlt.xRight - sl.rclBitBlt.xLeft))
                          / (ptlTopRight.x - ptlBotLeft.x);
            sizl.cy = 2 + ((LONG)bmpBitmapFile.cy
                           * (sl.rclBitBlt.yTop - sl.rclBitBlt.yBottom))
                          / (ptlTopRight.y - ptlBotLeft.y);

            bmp    = bmpBitmapFile;
            bmp.cx = LOUSHORT(sizl.cx);
            bmp.cy = LOUSHORT(sizl.cy);

            sl.hdcFill = DevOpenDC(habMain, OD_MEMORY, "*", 3L,
                                   (PDEVOPENDATA)&dop, NULLHANDLE);
            sl.hpsFill = GpiCreatePS(habMain, sl.hdcFill, &sizl,
                                     PU_PELS | GPIA_ASSOC | GPIT_MICRO);
            sl.hbmFill = GpiCreateBitmap(sl.hpsFill, (PBITMAPINFOHEADER2)&bmp, 0L, NULL, NULL);

            sl.hdcHole = DevOpenDC(habMain, OD_MEMORY, "*", 3L,
                                   (PDEVOPENDATA)&dop, NULLHANDLE);
            sl.hpsHole = GpiCreatePS(habMain, sl.hdcHole, &sizl,
                                     PU_PELS | GPIA_ASSOC | GPIT_MICRO);
            sl.hbmHole = GpiCreateBitmap(sl.hpsHole, (PBITMAPINFOHEADER2)&bmp, 0L, NULL, NULL);

            GpiSetSegmentAttrs(hpsClient, lLastSegId, ATTR_DETECTABLE, ATTR_ON);

            sl.lSegId         = lLastSegId;
            sl.pslNext        = NULL;
            sl.pslPrev        = NULL;
            sl.fIslandMark    = FALSE;
            sl.ptlModelXlate.x = sl.ptlModelXlate.y = 0L;
            SetRect(&sl);
            psl = SegListUpdate(ADD_TAIL_SEG, &sl);
            psl->pslNextIsland = psl;
        }
    }
    return TRUE;
}

BOOL
PrepareBitmap(VOID)
{
    hbmBitmapSize = GpiCreateBitmap(hpsBitmapSize, (PBITMAPINFOHEADER2)&bmpBitmapFile,
                                    0L, NULL, NULL);
    if (!hbmBitmapSize) return FALSE;

    bmpBitmapSave    = bmpBitmapFile;
    bmpBitmapSave.cx = LOUSHORT(sizlMaxClient.cx);
    bmpBitmapSave.cy = LOUSHORT(sizlMaxClient.cy);
    hbmBitmapSave    = GpiCreateBitmap(hpsBitmapSave, (PBITMAPINFOHEADER2)&bmpBitmapSave,
                                       0L, NULL, NULL);
    if (!hbmBitmapSave) return FALSE;
    GpiSetBitmap(hpsBitmapSave, hbmBitmapSave);

    hbmBitmapBuff = GpiCreateBitmap(hpsBitmapBuff, (PBITMAPINFOHEADER2)&bmpBitmapSave,
                                    0L, NULL, NULL);
    if (!hbmBitmapBuff) return FALSE;
    GpiSetBitmap(hpsBitmapBuff, hbmBitmapBuff);

    return TRUE;
}

BOOL
CreateBitmapHdcHps(HDC *phdc, HPS *phps)
{
    SIZEL sizl;
    HDC   hdc;
    HPS   hps;

    hdc = DevOpenDC(habMain, OD_MEMORY, "*", 3L,
                    (PDEVOPENDATA)&dop, NULLHANDLE);
    if (!hdc) return FALSE;

    sizl.cx = sizl.cy = 0L;
    hps = GpiCreatePS(habMain, hdc, &sizl,
                      PU_PELS | GPIA_ASSOC | GPIT_MICRO);
    if (!hps) return FALSE;

    *phdc = hdc;
    *phps = hps;
    return TRUE;
}

BOOL
ReadBitmap(PCHAR szFileName)
{
    HFILE  hfile;
    ULONG  ulAction, ulSize, cbRead;
    PVOID  pImage;
    FILESTATUS3     fsts;
    PBITMAPFILEHEADER pbfh;
    PRCBITMAP        rb;
    ULONG  cScans;
    BOOL   fRet = FALSE;

    if (DosOpen(szFileName, &hfile, &ulAction, 0, FILE_NORMAL,
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE |
                OPEN_ACCESS_READONLY, NULL) != 0)
        return FALSE;

    if (DosQueryFileInfo(hfile, FIL_STANDARD, &fsts, sizeof(fsts)) != 0)
        goto rb_close;

    ulSize = fsts.cbFile;
    if (DosAllocMem(&pImage, ulSize,
                    PAG_READ | PAG_WRITE | PAG_COMMIT) != 0)
        goto rb_close;

    if (DosRead(hfile, pImage, ulSize, &cbRead) != 0 || cbRead != ulSize)
        goto rb_free;

    rb   = (PRCBITMAP)pImage;
    pbfh = (PBITMAPFILEHEADER)pImage;

    if (pbfh->bmp.cbFix != sizeof(BITMAPINFOHEADER)) {
        /* old RCBITMAP format */
        bmpBitmapFile.cx       = rb->bmWidth;
        bmpBitmapFile.cy       = rb->bmHeight;
        bmpBitmapFile.cPlanes  = rb->bmPlanes;
        bmpBitmapFile.cBitCount= rb->bmBitcount;
        hbmBitmapFile = GpiCreateBitmap(hpsBitmapFile, (PBITMAPINFOHEADER2)&bmpBitmapFile,
                                        0L, NULL, NULL);
        if (!hbmBitmapFile) goto rb_free;
        GpiSetBitmap(hpsBitmapFile, hbmBitmapFile);
        {
            PBYTE pBits = (PBYTE)pImage + rb->dwBitsOffset;
            rb->dwBitsOffset = sizeof(BITMAPINFOHEADER);
            cScans = GpiSetBitmapBits(hpsBitmapFile, 0L, (LONG)rb->bmHeight,
                                      pBits,
                                      (PBITMAPINFO2)&(rb->dwBitsOffset));
            if (cScans != (LONG)rb->bmHeight) goto rb_free;
        }
    } else {
        /* standard BITMAPFILEHEADER format */
        bmpBitmapFile.cx       = pbfh->bmp.cx;
        bmpBitmapFile.cy       = pbfh->bmp.cy;
        bmpBitmapFile.cPlanes  = pbfh->bmp.cPlanes;
        bmpBitmapFile.cBitCount= pbfh->bmp.cBitCount;
        hbmBitmapFile = GpiCreateBitmap(hpsBitmapFile, (PBITMAPINFOHEADER2)&bmpBitmapFile,
                                        0L, NULL, NULL);
        if (!hbmBitmapFile) goto rb_free;
        GpiSetBitmap(hpsBitmapFile, hbmBitmapFile);
        cScans = GpiSetBitmapBits(hpsBitmapFile, 0L, (LONG)pbfh->bmp.cy,
                                  (PBYTE)pImage + pbfh->offBits,
                                  (PBITMAPINFO2)&pbfh->bmp);
        if (cScans != (LONG)pbfh->bmp.cy) goto rb_free;
    }
    fRet = TRUE;

rb_free:
    DosFreeMem(pImage);
rb_close:
    DosClose(hfile);
    return fRet;
}
