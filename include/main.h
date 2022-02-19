#ifndef GUARD_MAIN_H
#define GUARD_MAIN_H

typedef void (*MainCallback)(void);
typedef void (*IntrCallback)(void);
typedef void (*IntrFunc)(void);

#include "global.h"

extern IntrFunc gIntrTable[];

struct Main
{
    /*0x000*/ MainCallback callback1;
    /*0x004*/ MainCallback callback2;

    /*0x008*/ MainCallback savedCallback;

    /*0x00C*/ IntrCallback vblankCallback;
    /*0x010*/ IntrCallback hblankCallback;
    /*0x014*/ IntrCallback vcountCallback;
    /*0x018*/ IntrCallback serialCallback;

    /*0x01C*/ vu16 intrCheck;

    /*0x020*/ u32 *vblankCounter1;
    /*0x024*/ u32 vblankCounter2;

    /*0x026*/ u16 heldKeys;              // held keys
    /*0x028*/ u16 newKeys;               // newly pressed keys
    /*0x02A*/ u16 newAndRepeatedKeys;    // newly pressed keys plus key repeat
    /*0x02C*/ u16 keyRepeatCounter;      // counts down to 0, triggering key repeat
    /*0x030*/ bool16 watchedKeysPressed; // whether one of the watched keys was pressed
    /*0x032*/ u16 watchedKeysMask;       // bit mask for watched keys

    /*0x034*/ struct OamData oamBuffer[128];

    /*0x434*/ u8 state;

    /*0x435*/ u8 oamLoadDisabled:1;
};

extern struct Main gMain;
extern bool8 gSoftResetDisabled;

void AgbMain(void);
void SetMainCallback2(MainCallback callback);
void InitKeys(void);
void SetVBlankCallback(IntrCallback callback);
void SetHBlankCallback(IntrCallback callback);
void SetVCountCallback(IntrCallback callback);
void SetSerialCallback(IntrCallback callback);
void InitFlashTimer(void);
void DoSoftReset(void);
void RestoreSerialTimer3IntrHandlers(void);
void SetVBlankCounter1Ptr(u32 *ptr);
void DisableVBlankCounter1(void);
void StartTimer1(void);

extern const char RomHeaderGameCode[4];
extern const char RomHeaderSoftwareVersion;

extern u8 gLinkTransferringData;
extern u16 gKeyRepeatStartDelay;

#endif // GUARD_MAIN_H
