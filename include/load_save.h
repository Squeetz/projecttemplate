#ifndef GUARD_LOAD_SAVE_H
#define GUARD_LOAD_SAVE_H

#include "global.h"

extern bool32 gFlashMemoryPresent;
extern struct SaveBlock1 gSaveBlock1;
extern struct SaveBlock2 gSaveBlock2;

void ClearSav2(void);
void ClearSav1(void);
void CheckForFlashMemory(void);
void SaveSerializedGame(void);
void LoadSerializedGame(void);
void ApplyNewEncryptionKeyToAllEncryptedData(u32 encryptionKey);

#endif // GUARD_LOAD_SAVE_H
