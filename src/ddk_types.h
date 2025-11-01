#ifndef __DDK_TYPES_H_
#define __DDK_TYPES_H_


#define zxkiFMT_A8R8G8B8          21
#define zxkiFMT_X8R8G8B8          22
#define zxkiFMT_R5G6B5            23
/* from zxkmdt.h*/

typedef unsigned int zx_handle_t;
typedef unsigned int zx_context_t;

#define ALIGN8 __attribute__ ((aligned(8)))

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

typedef unsigned long long zx_ptr64_t;
typedef enum _zxKMDT_STANDARDALLOCATION_TYPE
{
    zxKMDT_STANDARDALLOCATION_SHAREDPRIMARYSURFACE  = 1,
    zxKMDT_STANDARDALLOCATION_SHADOWSURFACE         = 2,
    zxKMDT_STANDARDALLOCATION_STAGINGSURFACE        = 3,
} zxKMDT_STANDARDALLOCATION_TYPE;

typedef enum zxki_FLIPINTERVAL_TYPE
{
    zxki_FLIPINTERVAL_IMMEDIATE = 0,
    zxki_FLIPINTERVAL_ONE       = 1,
    zxki_FLIPINTERVAL_TWO       = 2,
    zxki_FLIPINTERVAL_THREE     = 3,
    zxki_FLIPINTERVAL_FOUR      = 4,
} zxki_FLIPINTERVAL_TYPE;


typedef struct _zxki_ALLOCATIONLIST
{
    unsigned int       hAllocation;
    union
    {
        struct
        {
            unsigned int     WriteOperation       : 1;    // 0x00000001
            unsigned int     DoNotRetireInstance : 1;    // 0x00000002            
            unsigned int     Reserved             :30;    // 0xFFFFFFFC
        };
        unsigned int         Value;
    };
} zxki_ALLOCATIONLIST;

typedef struct _zxki_PATCHLOCATIONLIST
{
    unsigned int     AllocationIndex;
    union
    {
        struct
        {
            unsigned int SlotId          : 24;   // 0x00FFFFFF
            unsigned int Reserved        : 8;    // 0xFF000000
        };
        unsigned int     Value;
    };
    unsigned int     DriverId;
    unsigned int     AllocationOffset;
    unsigned int     PatchOffset;
    unsigned int     SplitOffset;
} zxki_PATCHLOCATIONLIST;

typedef struct _zxki_SYNCOBJECTLIST
{
    unsigned int       hSyncObject;
    unsigned int       PatchOffset;
    unsigned long long FenceValue;
} zxki_SYNCOBJECTLIST;

typedef struct _zxkiCB_LOCKFLAGS
{
    union
    {
        struct
        {
            unsigned int ReadOnly        :1;
            unsigned int WriteOnly       :1;
            unsigned int DonotWait       :1;
            unsigned int IgnoreSync      :1;
            unsigned int LockEntire      :1;
            unsigned int DonotEvict      :1;
            unsigned int AcquireApeture  :1;
            unsigned int Discard         :1;
            unsigned int NoExistingReference        :1;
            unsigned int UseAlternateVA  :1;
            unsigned int IgnoreReadSync  :1;
            unsigned int Reserved_0      :2;
            unsigned int CacheType       :3;
            unsigned int Reserved        :16;
        };
        unsigned int value;
         
    };
}zxkiCB_LOCKFLAGS;


typedef struct _zxki_OPENALLOCATIONINFO
{
    unsigned int   hAllocation;                // HANDLE for this allocation in this process
    const void*    pPrivateDriverData;         // Ptr to driver private buffer for this allocations
    unsigned int   PrivateDriverDataSize;      // Size in bytes of driver private buffer for this allocations    
}zxki_OPENALLOCATIONINFO;

typedef struct _zxki_ALLOCATIONINFO
{
    unsigned int      Size;                  // out: allocation size
    unsigned int      hAllocation;           // out: Private driver data for allocation
    zx_ptr64_t       pSystemMem;            // in: Pointer to pre-allocated sysmem
    zx_ptr64_t       pPrivateDriverData;    // in(out optional): Private data for each allocation
    unsigned int      PrivateDriverDataSize; // in: Size of the private data
    unsigned int      VidPnSourceId;         // in: VidPN source ID if this is a primary
    unsigned int      __pad;
    union
    {
        struct
        {
            unsigned int    Primary         : 1;    // 0x00000001
            unsigned int    Reserved        :31;    // 0xFFFFFFFE
        };
        unsigned int        Value;
    } Flags;
} zxki_ALLOCATIONINFO;

// according to zxkiCB_RENDERFLAGS
typedef struct _KMD_RENDERFLAGS
{
    union
    {
        struct
        {
            unsigned int    ResizeCommandBuffer      : 1;     // 0x00000001
            unsigned int    ResizeAllocationList     : 1;     // 0x00000002
            unsigned int    ResizePatchLocationList  : 1;     // 0x00000004
            unsigned int    NullRendering            : 1;     // 0x00000008
            unsigned int    DumpMiu                  : 1;     // 0x00000010
            unsigned int    Reserved                 :27;     // 0xFFFFFFE0
        };
        unsigned int Value;
    };
} KMD_RENDERFLAGS;

typedef struct _zxKMT_PRESENTFLAGS
{
    union
    {
        struct
        {
            unsigned int Blt                 : 1;        // 0x00000001
            unsigned int ColorFill           : 1;        // 0x00000002
            unsigned int Flip                : 1;        // 0x00000004
            unsigned int FlipDoNotFlip       : 1;        // 0x00000008
            unsigned int FlipDoNotWait       : 1;        // 0x00000010
            unsigned int FlipRestart         : 1;        // 0x00000020
            unsigned int DstRectValid        : 1;        // 0x00000040
            unsigned int SrcRectValid        : 1;        // 0x00000080
            unsigned int RestrictVidPnSource : 1;        // 0x00000100
            unsigned int SrcColorKey         : 1;        // 0x00000200
            unsigned int DstColorKey         : 1;        // 0x00000400
            unsigned int LinearToSrgb        : 1;        // 0x00000800
            unsigned int PresentCountValid   : 1;        // 0x00001000
            unsigned int Rotate              : 1;        // 0x00002000
            unsigned int Reserved            :18;        // 0xFFFFC000
        };
        unsigned int Value;
    };
} zxKMT_PRESENTFLAGS;

typedef struct __zxki_PresentFlagsRec{
    union
    {
        struct
        {
            unsigned int    Blt                 : 1;        // 0x00000001
            unsigned int    ColorFill           : 1;        // 0x00000002
            unsigned int    Flip                : 1;        // 0x00000004
            unsigned int    FlipDoNotFlip       : 1;        // 0x00000008
            unsigned int    FlipDoNotWait       : 1;        // 0x00000010 
            unsigned int    FlipRestart         : 1;        // 0x00000020
            unsigned int    DstRectValid        : 1;        // 0x00000040
            unsigned int    SrcRectValid        : 1;        // 0x00000080
            unsigned int    RestrictVidPnSource : 1;        // 0x00000100
            unsigned int    SrcColorKey         : 1;        // 0x00000200
            unsigned int    DstColorKey         : 1;        // 0x00000400
            unsigned int    LinearToSrgb        : 1;        // 0x00000800
            unsigned int    PresentCountValid   : 1;        // 0x00001000
            unsigned int    Rotate              : 1;        // 0x00002000
            unsigned int    VideoFlip            : 1;
            unsigned int    Reserved            :17;        // 0xFFFFC000
        };
        unsigned int    Value;
    };
} zxki_PresentFlags;

typedef struct _zxKMT_CREATEALLOCATIONFLAGS
{
    unsigned int CreateResource           :1;
    unsigned int CreateShared             :1;
    unsigned int NonSecure                :1;
    unsigned int Reserved                 :29;
}zxKMT_CREATEALLOCATIONFLAGS;

/*Declaration must match with UMD*/
enum {
    INTERNAL_PRESENT_SYNCFRONT      = 1,
    INTERNAL_PRESENT_BLTALLOCATION  = 2,
    INTERNAL_PRESENT_RESTOREFRONT   = 3,
    INTERNAL_PRESENT_SYNCTOFRONT    = 4,
};


#endif

