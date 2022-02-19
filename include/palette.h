#ifndef GUARD_PALETTE_H
#define GUARD_PALETTE_H

#include "global.h"

#define gPaletteFade_selectedPalettes (gPaletteFade.multipurpose1) // normal and fast fade
#define gPaletteFade_blendCnt         (gPaletteFade.multipurpose1) // hardware fade
#define gPaletteFade_delay            (gPaletteFade.multipurpose2) // normal and hardware fade
#define gPaletteFade_submode          (gPaletteFade.multipurpose2) // fast fade

#define PLTT_BUFFER_SIZE 0x200
#define PLTT_DECOMP_BUFFER_SIZE (PLTT_BUFFER_SIZE * 2)

#define PALETTE_FADE_STATUS_DELAY 2
#define PALETTE_FADE_STATUS_ACTIVE 1
#define PALETTE_FADE_STATUS_DONE 0
#define PALETTE_FADE_STATUS_LOADING 0xFF

#define PALETTES_BG      0x0000FFFF
#define PALETTES_OBJECTS 0xFFFF0000
#define PALETTES_ALL     (PALETTES_BG | PALETTES_OBJECTS)

enum
{
    FAST_FADE_IN_FROM_WHITE,
    FAST_FADE_OUT_TO_WHITE,
    FAST_FADE_IN_FROM_BLACK,
    FAST_FADE_OUT_TO_BLACK,
};

struct PaletteFadeControl // size = 0x10
{
    /* 0x00 */ u32 multipurpose1;
    /* 0x04 */ u16 blendcolor:15;
    /* 0x04 */ bool16 active:1;
    /* 0x06 */ u16 y; // blend coefficient
    /* 0x08 */ u16 denominator;
    /* 0x0A */ u8 targetY:6 //target blend coefficient
    /* 0x0A */ u8 mode:2;
    /* 0x0B */ u8 deltaY:6 //rate of change of blend coefficient
    /* 0x0B */ bool8 yChanged:1;
    /* 0x0B */ bool8 yDec:1; // whether blend coefficient is decreasing
    /* 0x0C */ u8 delayCounter:6;
    /* 0x0C */ bool8 hardwareFadeFinishing:1;
    /* 0x0C */ bool8 doEndDelay:1;
    /* 0x0D */ u8 multipurpose2:6;
    /* 0x0D */ bool8 objPaletteToggle:1;
    /* 0x0D */ bool8 shouldResetBlendRegisters:1;
    /* 0x0E */ u8 softwareFadeFinishingCounter:3;
    /* 0x0E */ bool8 bufferTransferDisabled:1;
    /* 0x0F */ u8 unused;
};

extern struct PaletteFadeControl gPaletteFade;
extern u32 gPlttBufferTransferPending;
extern u16 gPlttBufferUnfaded[PLTT_BUFFER_SIZE];
extern u16 gPlttBufferFaded[PLTT_BUFFER_SIZE];

void LoadCompressedPalette(const u32 *src, u16 offset, u16 size);
void LoadPalette(const void *src, u16 offset, u16 size);
void FillPalette(u16 value, u16 offset, u16 size);
void TransferPlttBuffer(void);
u8 UpdatePaletteFade(void);
void ResetPaletteFade(void);
void ReadPlttIntoBuffers(void);
void ResetPaletteFadedBuffer(u32 selectedPalettes);
void CopyFadedIntoUnfadedBuffer(u32 selectedPalettes);
bool8 BeginNormalPaletteFade(u32 selectedPalettes, s8 delay, u8 startY, u8 targetY, u16 blendColor);
bool8 BeginNormalPaletteFadeForDuration(u32 selectedPalettes, u16 fadeDuration, u8 startY, u8 targetY, u16 blendColor, u32 doEndDelay);
void BlendPalettesFine(u32 selectedPalettes, u32 coeff, u32 color);
void InvertPlttBuffer(u32 selectedPalettes);
void TintPlttBuffer(u32 selectedPalettes, s8 r, s8 g, s8 b);
void UnfadePlttBuffer(u32 selectedPalettes);
void BeginFastPaletteFade(u8 submode);
void BeginHardwarePaletteFade(u8 blendCnt, u8 delay, u8 y, u8 targetY, u8 shouldResetBlendRegisters);
void BlendPalettes(u32 selectedPalettes, u8 coeff, u16 color);
void BlendPalettesUnfaded(u32 selectedPalettes, u8 coeff, u16 color);
void TintPalette_GrayScale(u16 *palette, u16 count);
void TintPalette_GrayScale2(u16 *palette, u16 count);
void TintPalette_SepiaTone(u16 *palette, u16 count);
void TintPalette_CustomTone(u16 *palette, u16 count, u16 rTone, u16 gTone, u16 bTone);

#endif // GUARD_PALETTE_H
