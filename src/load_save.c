#include "global.h"
#include "gflib.h"
#include "gba/flash_internal.h"
#include "load_save.h"
#include "main.h"
#include "random.h"

#define SAVEBLOCK_MOVE_RANGE    128

// EWRAM DATA
EWRAM_DATA struct SaveBlock2 gSaveBlock2 = {0};
EWRAM_DATA u8 gSaveBlock2_DMA[SAVEBLOCK_MOVE_RANGE] = {0};

EWRAM_DATA struct SaveBlock1 gSaveBlock1 = {0};
EWRAM_DATA u8 gSaveBlock1_DMA[SAVEBLOCK_MOVE_RANGE] = {0};

EWRAM_DATA u32 gLastEncryptionKey = 0;

// IWRAM common
bool32 gFlashMemoryPresent;
struct SaveBlock1 *gSaveBlock1Ptr;
struct SaveBlock2 *gSaveBlock2Ptr;

void CheckForFlashMemory(void)
{
    if (!IdentifyFlash())
    {
        gFlashMemoryPresent = TRUE;
        InitFlashTimer();
    }
    else
    {
        gFlashMemoryPresent = FALSE;
    }
}

void ClearSav2(void)
{
    CpuFill16(0, &gSaveBlock2, sizeof(struct SaveBlock2) + sizeof(gSaveBlock2_DMA));
}

void ClearSav1(void)
{
    CpuFill16(0, &gSaveBlock1, sizeof(struct SaveBlock1) + sizeof(gSaveBlock1_DMA));
}

void SetSaveBlocksPointers(void)
{
    u32 offset;
    struct SaveBlock1** sav1_LocalVar = &gSaveBlock1Ptr;
    void *oldSave = (void *)gSaveBlock1Ptr;

    offset = (Random()) & ((SAVEBLOCK_MOVE_RANGE - 1) & ~3);

    gSaveBlock2Ptr = (void*)(&gSaveBlock2) + offset;
    *sav1_LocalVar = (void*)(&gSaveBlock1) + offset;
}

void MoveSaveBlocks_ResetHeap(void)
{
    void *vblankCB, *hblankCB;
    u32 encryptionKey;
    struct SaveBlock2 *saveBlock2Copy;
    struct SaveBlock1 *saveBlock1Copy;

    // save interrupt functions and turn them off
    vblankCB = gMain.vblankCallback;
    hblankCB = gMain.hblankCallback;
    gMain.vblankCallback = NULL;
    gMain.hblankCallback = NULL;
    gMain.vblankCounter1 = NULL;
    
    saveBlock2Copy = (struct SaveBlock2 *)(gHeap);
    saveBlock1Copy = (struct SaveBlock1 *)(gHeap + sizeof(struct SaveBlock2));

    // backup the saves.
    *saveBlock2Copy = *gSaveBlock2Ptr;
    *saveBlock1Copy = *gSaveBlock1Ptr;

    // change saveblocks' pointers
    SetSaveBlocksPointers();

    // restore saveblock data since the pointers changed
    *gSaveBlock2Ptr = *saveBlock2Copy;
    *gSaveBlock1Ptr = *saveBlock1Copy;

    // heap was destroyed in the copying process, so reset it
    InitHeap(gHeap, HEAP_SIZE);

    // restore interrupt functions
    gMain.hblankCallback = hblankCB;
    gMain.vblankCallback = vblankCB;

    // create a new encryption key
    encryptionKey = (Random() << 0x10) + (Random());
    gSaveBlock2Ptr->encryptionKey = encryptionKey;
}

void ApplyNewEncryptionKeyToHword(u16 *hWord, u32 newKey)
{
    *hWord ^= gSaveBlock2Ptr->encryptionKey;
    *hWord ^= newKey;
}

void ApplyNewEncryptionKeyToWord(u32 *word, u32 newKey)
{
    *word ^= gSaveBlock2Ptr->encryptionKey;
    *word ^= newKey;
}
