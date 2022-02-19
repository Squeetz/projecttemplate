#include "global.h"
#include "gflib.h"
#include "util.h"
#include "decompress.h"
#include "task.h"

enum
{
    NORMAL_FADE,
    FAST_FADE,
    HARDWARE_FADE,
};

static u8 GetPaletteNumByUid(u16);
static u8 UpdateNormalPaletteFade(void);
static void BeginFastPaletteFadeInternal(u8);
static u8 UpdateFastPaletteFade(void);
static u8 UpdateHardwarePaletteFade(void);
static void UpdateBlendRegisters(void);
static bool8 IsSoftwarePaletteFadeFinishing(void);
static bool8 BeginNormalPaletteFadeInternal(u32 selectedPalettes, u8 startY, u8 targetY, u16 blendColor, u32 denominator, u32 doEndDelay);
static void CopyPalBuffer1IntoPalBuffer2(u32 selectedPalettes, u16 * palDataSrc, u16 * palDataDst);

ALIGNED(4) EWRAM_DATA u16 gPlttBufferUnfaded[PLTT_BUFFER_SIZE] = {0};
ALIGNED(4) EWRAM_DATA u16 gPlttBufferFaded[PLTT_BUFFER_SIZE] = {0};
EWRAM_DATA struct PaletteFadeControl gPaletteFade = {0};
static EWRAM_DATA u32 sPlttBufferTransferPending = 0;
EWRAM_DATA u8 gPaletteDecompressionBuffer[PLTT_DECOMP_BUFFER_SIZE] = {0};

static const u8 sRoundedDownGrayscaleMap[] =
{
     0,  0,  0,  0,  0,
     5,  5,  5,  5,  5,
    11, 11, 11, 11, 11,
    16, 16, 16, 16, 16,
    21, 21, 21, 21, 21,
    27, 27, 27, 27, 27,
    31, 31
};

void LoadCompressedPalette(const u32 *src, u16 offset, u16 size)
{
    LZDecompressWram(src, gPaletteDecompressionBuffer);
    CpuCopy16(gPaletteDecompressionBuffer, gPlttBufferUnfaded + offset, size);
    CpuCopy16(gPaletteDecompressionBuffer, gPlttBufferFaded + offset, size);
}

void LoadPalette(const void *src, u16 offset, u16 size)
{
    CpuCopy16(src, gPlttBufferUnfaded + offset, size);
    CpuCopy16(src, gPlttBufferFaded + offset, size);
}

void FillPalette(u16 value, u16 offset, u16 size)
{
    CpuFill16(value, gPlttBufferUnfaded + offset, size);
    CpuFill16(value, gPlttBufferFaded + offset, size);
}

void TransferPlttBuffer(void)
{
    if (!gPaletteFade.bufferTransferDisabled)
    {
        void *src = gPlttBufferFaded;
        void *dest = (void *)PLTT;
        DmaCopy16(3, src, dest, PLTT_SIZE);
        sPlttBufferTransferPending = 0;
        if (gPaletteFade.mode == HARDWARE_FADE && gPaletteFade.active)
            UpdateBlendRegisters();
    }
}

u8 UpdatePaletteFade(void)
{
    u8 result;

    if (sPlttBufferTransferPending)
        return PALETTE_FADE_STATUS_LOADING;
	
    if (gPaletteFade.mode == NORMAL_FADE)
        result = UpdateNormalPaletteFade();
    else if (gPaletteFade.mode == FAST_FADE)
        result = UpdateFastPaletteFade();
    else
        result = UpdateHardwarePaletteFade();
	
    sPlttBufferTransferPending = gPaletteFade.multipurpose1;
    
    return result;
}

void ReadPlttIntoBuffers(void)
{
    u16 i;
    u16 *pltt = (u16 *)PLTT;

    for (i = 0; i < PLTT_SIZE / 2; ++i)
    {
        gPlttBufferUnfaded[i] = pltt[i];
        gPlttBufferFaded[i] = pltt[i];
    }
}

void ResetPaletteFadedBuffer(u32 selectedPalettes)
{
    CopyPalBuffer1IntoPalBuffer2(selectedPalettes, gPlttBufferUnfaded, gPlttBufferFaded);
}

void CopyFadedIntoUnfadedBuffer(u32 selectedPalettes)
{
    CopyPalBuffer1IntoPalBuffer2(selectedPalettes, gPlttBufferFaded, gPlttBufferUnfaded);
}

static void CopyPalBuffer1IntoPalBuffer2(u32 selectedPalettes, u16 *src, u16 *dst)
{
    int savedBufferTransferDisabled = gPaletteFade.bufferTransferDisabled;
    
    ResetPaletteFadeControl();
    
    gPaletteFade.bufferTransferDisabled = TRUE;
    
    while (selectedPalettes)
    {
        if (selectedPalettes & 0x1)
	    CpuCopy16(src, dst, 0x20);
	    
	src += 16;
        dst += 16;
        selectedPalettes >>= 1;
    }
    
    gPaletteFade.bufferTransferDisabled = savedBufferTransferDisabled;
}

bool8 BeginNormalPaletteFade(u32 selectedPalettes, s8 delay, u8 startY, u8 targetY, u16 blendColor)
{
    u8 diff;
    u16 denominator;
    
    if (startY < targetY)
        diff = targetY - startY;
    else
        diff = startY - targetY;

    if (delay >= 0)
    {
        if (delay > 63)
	    delay = 63;
	    
	denominator = ((diff + 1) / 2) * (delay + 2) + 1;
    }
    else
    {
        u32 i, deltaY;
	s8 y;
	
	if (delay < -14)
	    delay = -14;
	    
	deltaY = 2 + (delay * -1);
	y = startY;
	
	if (y < targetY)
	{
	    for (i = 0; y < targetY; i++)
	        y += deltaY;
	}
	else
	{
	    for (i = 0; y > targetY; i++)
	        y -= deltaY;
	}
	
	denominator = i * 2 + 1;
    }
    
    return BeginNormalPaletteFadeInternal(selectedPalettes, startY * 2, targetY * 2, blendColor, denominator, TRUE);
}

bool8 BeginNormalPaletteFadeForDuration(u32 selectedPalettes, u16 fadeDuration, u8 startY, u8 targetY, u16 blendColor, u32 doEndDelay)
{
    if (fadeDuration > 1)
        fadeDuration--;
    else
        fadeDuration = 1;
	
    return BeginNormalPaletteFadeInternal(selectedPalettes, startY, targetY, blendColor, fadeDuration, doEndDelay);
}

static bool8 BeginNormalPaletteFadeInternal(u32 selectedPalettes, u8 startY, u8 targetY, u16 blendColor, u32 denominator, u32 doEndDelay)
{
    u8 temp;
    u16 color = blendColor;
    
    if (gPaletteFade.active)
        return FALSE;
	
    gPaletteFade_selectedPalettes = selectedPalettes;
    gPaletteFade.y = startY * denominator;
    gPaletteFade.targetY = targetY;
    gPaletteFade.denominator = denominator;
    gPaletteFade.blendColor = color;
    gPaletteFade.active = TRUE;
    gPaletteFade.mode = NORMAL_FADE;
    gPaletteFade.objPaletteToggle = 0;
    gPaletteFade.yChanged = TRUE;
    gPaletteFade.doEndDelay = doEndDelay;
    
    if (startY < targetY)
    {
        gPaletteFade.deltaY = targetY - startY;
	gPaletteFade.yDec = FALSE;
    }
    else
    {
        gPaletteFade.deltaY = startY - targetY;
	gPaletteFade.yDec = TRUE;
    }
    
    UpdatePaletteFade();
    
    temp = gPaletteFade.bufferTransferDisabled;
    gPaletteFade.bufferTransferDisabled = 0;
    CpuCopy32(gPlttBufferFaded, (void *)PLTT, PLTT_SIZE);
    sPlttBufferTransferPending = 0;
    if (gPaletteFade.mode == HARDWARE_FADE && gPaletteFade.active)
        UpdateBlendRegisters();
    
    gPaletteFade.bufferTransferDisabled = temp;
    return TRUE;
}

void ResetPaletteFade(void)
{
    gPaletteFade.multipurpose1 = 0;
    gPaletteFade.multipurpose2 = 0;
    gPaletteFade.delayCounter = 0;
    gPaletteFade.y = 0;
    gPaletteFade.denominator = 0;
    gPaletteFade.targetY = 0;
    gPaletteFade.blendColor = 0;
    gPaletteFade.active = FALSE;
    gPaletteFade.yDec = FALSE;
    gPaletteFade.bufferTransferDisabled = FALSE;
    gPaletteFade.shouldResetBlendRegisters = FALSE;
    gPaletteFade.hardwareFadeFinishing = FALSE;
    gPaletteFade.softwareFadeFinishing = FALSE;
    gPaletteFade.softwareFadeFinishingCounter = 0;
    gPaletteFade.objPaletteToggle = 0;
    gPaletteFade.deltaY = 2;
}

static u8 UpdateNormalPaletteFade(void)
{
    if (!gPaletteFade.active)
        return PALETTE_FADE_STATUS_DONE;
	
    if (IsSoftwarePaletteFadeFinishing())
        return gPaletteFade.active ? PALETTE_FADE_STATUS_ACTIVE : PALETTE_FADE_STATUS_DONE;
	
    u16 targetY = gPaletteFade.targetY * gPaletteFade.denominator;
    u16 yWholeValue = gPaletteFade.y / gPaletteFade.denominator;
    
    if (gPaletteFade.yChanged)
        BlendPalettesFine(gPaletteFade_selectedPalettes, yWholeValue, gPaletteFade.blendColor);
	
    if (gPaletteFade.y == targetY)
    {
        gPaletteFade_selectedPalettes = 0;
	gPaletteFade.softwareFadeFinishingCounter = 1;
    }
    else
    {
        u16 newY;
	
	if (!gPaletteFade.yDec)
	{
	    newY = gPaletteFade.y + gPaletteFade.deltaY;
	    
	    if (newY > targetY)
	        newY = targetY;
	}
	else
	{
	    newY = gPaletteFade.y - gPaletteFade.deltaY;
	    
	    if (newY < targetY)
	        newY = targetY;
	}
	
	gPaletteFade.yChanged = (newY / gPaletteFade.denominator) != yWholeValue;
	gPaletteFade.y = newY;
    }
    
    return PALETTE_FADE_STATUS_ACTIVE;
}

void InvertPlttBuffer(u32 selectedPalettes)
{
    u16 paletteOffset = 0;

    while (selectedPalettes)
    {
        if (selectedPalettes & 1)
        {
            u8 i;

            for (i = 0; i < 16; ++i)
                gPlttBufferFaded[paletteOffset + i] = ~gPlttBufferFaded[paletteOffset + i];
        }
	
        selectedPalettes >>= 1;
        paletteOffset += 16;
    }
}

void TintPlttBuffer(u32 selectedPalettes, s8 r, s8 g, s8 b)
{
    u16 paletteOffset = 0;

    while (selectedPalettes)
    {
        if (selectedPalettes & 1)
        {
            u8 i;

            for (i = 0; i < 16; ++i)
            {
                struct PlttData *data = (struct PlttData *)&gPlttBufferFaded[paletteOffset + i];
                
                data->r += r;
                data->g += g;
                data->b += b;
            }
        }
        selectedPalettes >>= 1;
        paletteOffset += 16;
    }
}

void UnfadePlttBuffer(u32 selectedPalettes)
{
    u16 paletteOffset = 0;

    while (selectedPalettes)
    {
        if (selectedPalettes & 1)
        {
            u8 i;

            for (i = 0; i < 16; ++i)
                gPlttBufferFaded[paletteOffset + i] = gPlttBufferUnfaded[paletteOffset + i];
        }
	
        selectedPalettes >>= 1;
        paletteOffset += 16;
    }
}

void BeginFastPaletteFade(u8 submode)
{
    gPaletteFade.deltaY = 2;
    BeginFastPaletteFadeInternal(submode);
}

static void BeginFastPaletteFadeInternal(u8 submode)
{
    gPaletteFade.y = 31;
    gPaletteFade_submode = submode & 0x3F;
    gPaletteFade.active = TRUE;
    gPaletteFade.mode = FAST_FADE;
    gPaletteFade.doEndDelay = TRUE;
    
    if (submode == FAST_FADE_IN_FROM_BLACK)
        CpuFill16(RGB_BLACK, gPlttBufferFaded, PLTT_SIZE);
	
    if (submode == FAST_FADE_IN_FROM_WHITE)
        CpuFill16(RGB_WHITE, gPlttBufferFaded, PLTT_SIZE);
	
    UpdatePaletteFade();
}

static u8 UpdateFastPaletteFade(void)
{
    u16 i;
    u16 paletteOffsetStart, paletteOffsetEnd;
    s8 r0, g0, b0, r, g, b;

    if (!gPaletteFade.active)
        return PALETTE_FADE_STATUS_DONE;
	
    if (IsSoftwarePaletteFadeFinishing())
        return gPaletteFade.active ? PALETTE_FADE_STATUS_ACTIVE : PALETTE_FADE_STATUS_DONE;
	
    if (gPaletteFade.objPaletteToggle)
    {
        paletteOffsetStart = 256;
        paletteOffsetEnd = 512;
    }
    else
    {
        paletteOffsetStart = 0;
        paletteOffsetEnd = 256;
    }
    
    switch (gPaletteFade_submode)
    {
    case FAST_FADE_IN_FROM_WHITE:
        for (i = paletteOffsetStart; i < paletteOffsetEnd; ++i)
        {
            struct PlttData *unfaded;
            struct PlttData *faded;

            unfaded = (struct PlttData *)&gPlttBufferUnfaded[i];
            r0 = unfaded->r;
            g0 = unfaded->g;
            b0 = unfaded->b;
	    
            faded = (struct PlttData *)&gPlttBufferFaded[i];
            r = faded->r - 2;
            g = faded->g - 2;
            b = faded->b - 2;
	    
            if (r < r0)
                r = r0;
            if (g < g0)
                g = g0;
            if (b < b0)
                b = b0;
		
            gPlttBufferFaded[i] = r | (g << 5) | (b << 10);
        }
        break;
    case FAST_FADE_OUT_TO_WHITE:
        for (i = paletteOffsetStart; i < paletteOffsetEnd; ++i)
        {
            struct PlttData *data = (struct PlttData *)&gPlttBufferFaded[i];
            r = data->r + 2;
            g = data->g + 2;
            b = data->b + 2;
	    
            if (r > 31)
                r = 31;
            if (g > 31)
                g = 31;
            if (b > 31)
                b = 31;
		
            gPlttBufferFaded[i] = r | (g << 5) | (b << 10);
        }
        break;
    case FAST_FADE_IN_FROM_BLACK:
        for (i = paletteOffsetStart; i < paletteOffsetEnd; ++i)
        {
            struct PlttData *unfaded;
            struct PlttData *faded;

            unfaded = (struct PlttData *)&gPlttBufferUnfaded[i];
            r0 = unfaded->r;
            g0 = unfaded->g;
            b0 = unfaded->b;
	    
            faded = (struct PlttData *)&gPlttBufferFaded[i];
            r = faded->r + 2;
            g = faded->g + 2;
            b = faded->b + 2;
	    
            if (r > r0)
                r = r0;
            if (g > g0)
                g = g0;
            if (b > b0)
                b = b0;
		
            gPlttBufferFaded[i] = r | (g << 5) | (b << 10);
        }
        break;
    case FAST_FADE_OUT_TO_BLACK:
        for (i = paletteOffsetStart; i < paletteOffsetEnd; ++i)
        {
            struct PlttData *data = (struct PlttData *)&gPlttBufferFaded[i];
            r = data->r - 2;
            g = data->g - 2;
            b = data->b - 2;
	    
            if (r < 0)
                r = 0;
            if (g < 0)
                g = 0;
            if (b < 0)
                b = 0;
		
            gPlttBufferFaded[i] = r | (g << 5) | (b << 10);
        }
    }
    
    gPaletteFade.objPaletteToggle ^= 1;
    
    if (gPaletteFade.objPaletteToggle)
        return PALETTE_FADE_STATUS_ACTIVE;
	
    if (gPaletteFade.y - gPaletteFade.deltaY < 0)
        gPaletteFade.y = 0;
    else
        gPaletteFade.y -= gPaletteFade.deltaY;
	
    if (gPaletteFade.y == 0)
    {
        switch (gPaletteFade_submode)
        {
        case FAST_FADE_IN_FROM_WHITE:
        case FAST_FADE_IN_FROM_BLACK:
            CpuCopy32(gPlttBufferUnfaded, gPlttBufferFaded, PLTT_SIZE);
            break;
        case FAST_FADE_OUT_TO_WHITE:
            CpuFill32(0xFFFFFFFF, gPlttBufferFaded, PLTT_SIZE);
            break;
        case FAST_FADE_OUT_TO_BLACK:
            CpuFill32(0x00000000, gPlttBufferFaded, PLTT_SIZE);
            break;
        }
        gPaletteFade.mode = NORMAL_FADE;
        gPaletteFade.softwareFadeFinishing = TRUE;
    }

    return PALETTE_FADE_STATUS_ACTIVE;
}

void BeginHardwarePaletteFade(u8 blendCnt, u8 delay, u8 y, u8 targetY, u8 shouldResetBlendRegisters)
{
    gPaletteFade_blendCnt = blendCnt;
    gPaletteFade.delayCounter = delay;
    gPaletteFade_delay = delay;
    gPaletteFade.y = y;
    gPaletteFade.targetY = targetY;
    gPaletteFade.active = TRUE;
    gPaletteFade.mode = HARDWARE_FADE;
    gPaletteFade.shouldResetBlendRegisters = shouldResetBlendRegisters & 1;
    gPaletteFade.hardwareFadeFinishing = FALSE;
    
    if (y < targetY)
        gPaletteFade.yDec = FALSE;
    else
        gPaletteFade.yDec = TRUE;
}

static u8 UpdateHardwarePaletteFade(void)
{
    if (!gPaletteFade.active)
        return PALETTE_FADE_STATUS_DONE;
	
    if (gPaletteFade.delayCounter < gPaletteFade_delay)
    {
        gPaletteFade.delayCounter++;
        return PALETTE_FADE_STATUS_DELAY;
    }
    
    gPaletteFade.delayCounter = 0;
    
    if (!gPaletteFade.yDec)
    {
        gPaletteFade.y++;
	
        if (gPaletteFade.y > gPaletteFade.targetY)
        {
            gPaletteFade.hardwareFadeFinishing = TRUE;
            gPaletteFade.y--;
        }
    }
    else
    {
        if (gPaletteFade.y-- - 1 < gPaletteFade.targetY)
        {
            gPaletteFade.hardwareFadeFinishing = TRUE;
            gPaletteFade.y++;
        }
    }

    if (gPaletteFade.hardwareFadeFinishing)
    {
        if (gPaletteFade.shouldResetBlendRegisters)
        {
            gPaletteFade_blendCnt = 0;
            gPaletteFade.y = 0;
        }
        gPaletteFade.shouldResetBlendRegisters = FALSE;
    }

    return PALETTE_FADE_STATUS_ACTIVE;
}

static void UpdateBlendRegisters(void)
{
    SetGpuReg(REG_OFFSET_BLDCNT, (u16)gPaletteFade_blendCnt);
    SetGpuReg(REG_OFFSET_BLDY, gPaletteFade.y);
    
    if (gPaletteFade.hardwareFadeFinishing)
    {
        gPaletteFade.hardwareFadeFinishing = FALSE;
        gPaletteFade.mode = 0;
        gPaletteFade_blendCnt = 0;
        gPaletteFade.y = 0;
        gPaletteFade.active = FALSE;
    }
}

static bool8 IsSoftwarePaletteFadeFinishing(void)
{
    if (!gPaletteFade.softwareFadeFinishingCounter)
        return FALSE;
    
    if (!gPaletteFade.doEndDelay || gPaletteFade.softwareFadeFinishingCounter == 5)
    {
        gPaletteFade.active = FALSE;
	gPaletteFade.softwareFadeFinishingCounter = 0;
    }
    else
        gPaletteFade.softwareFadeFinishingCounter++;
	
    return TRUE;
}

void BlendPalettes(u32 selectedPalettes, u8 coeff, u16 color)
{
    BlendPalettesFine(selectedPalettes, coeff * 2, color);
}

void BlendPalettesFine(u32 selectedPalettes, u32 coeff, u32 blendColor)
{
    s32, newR, newG, newB;
    u16 *src;
    u16 *dst;
    
    if (!selectedPalettes)
        return;
	
    newR = (blendColor << 27) >> 27;
    newG = (blendColor << 22) >> 27;
    newB = (blendColor << 17) >> 27;
    
    src = gPlttBufferUnfaded;
    dst = gPlttBufferFaded;
    
    do
    {
        if (selectedPalettes & 0x1)
	{
	    u16 *srcEnd = src + 16;
	    
	    while(src != srcEnd)
	    {
	        u32 srcColor = *src;
		
		s32 r = (srcColor << 27) >> 27;
		s32 g = (srcColor << 22) >> 27;
		s32 b = (srcColor << 17) >> 27;
		
		*dst = (r + (((newR - r) * coeff) >> 5))
		        | ((g + (((newG - g) * coeff) >> 5)) << 5)
			| ((b + (((newB - b) * coeff) >> 5)) << 10);
			
		src++;
		dst++;
	    }
	}
	else
	{
	    src += 16;
	    dst += 16;
	}
	
	selectedPalettes >>= 1;
    } while (selectedPalettes);
}

void BlendPalettesUnfaded(u32 selectedPalettes, u8 coeff, u16 color)
{
    CpuFastCopy(gPlttBufferUnfaded, gPlttBufferFaded, PLTT_SIZE);
    BlendPalettes(selectedPalettes, coeff, color);
}

void TintPalette_GrayScale(u16 *palette, u16 count)
{
    s32 r, g, b, i;
    u32 gray;

    for (i = 0; i < count; ++i)
    {
        r = (*palette >>  0) & 0x1F;
        g = (*palette >>  5) & 0x1F;
        b = (*palette >> 10) & 0x1F;
	
        gray = (r * Q_8_8(0.3) + g * Q_8_8(0.59) + b * Q_8_8(0.1133)) >> 8;
        *palette++ = (gray << 10) | (gray << 5) | (gray << 0);
    }
}

void TintPalette_GrayScale2(u16 *palette, u16 count)
{
    s32 r, g, b, i;
    u32 gray;

    for (i = 0; i < count; ++i)
    {
        r = (*palette >>  0) & 0x1F;
        g = (*palette >>  5) & 0x1F;
        b = (*palette >> 10) & 0x1F;
	
        gray = (r * Q_8_8(0.3) + g * Q_8_8(0.59) + b * Q_8_8(0.1133)) >> 8;

        if (gray > 0x1F)
            gray = 0x1F;
	    
        gray = sRoundedDownGrayscaleMap[gray];
        *palette++ = (gray << 10) | (gray << 5) | (gray << 0);
    }
}

void TintPalette_SepiaTone(u16 *palette, u16 count)
{
    s32 r, g, b, i;
    u32 gray;

    for (i = 0; i < count; ++i)
    {
        r = (*palette >>  0) & 0x1F;
        g = (*palette >>  5) & 0x1F;
        b = (*palette >> 10) & 0x1F;
	
        gray = (r * Q_8_8(0.3) + g * Q_8_8(0.59) + b * Q_8_8(0.1133)) >> 8;
	
        r = (u16)((Q_8_8(1.2) * gray)) >> 8;
        g = (u16)((Q_8_8(1.0) * gray)) >> 8;
        b = (u16)((Q_8_8(0.94) * gray)) >> 8;
	
        if (r > 31)
            r = 31;
	    
        *palette++ = (b << 10) | (g << 5) | (r << 0);
    }
}

void TintPalette_CustomTone(u16 *palette, u16 count, u16 rTone, u16 gTone, u16 bTone)
{
    s32 r, g, b, i;
    u32 gray;

    for (i = 0; i < count; ++i)
    {
        r = (*palette >>  0) & 0x1F;
        g = (*palette >>  5) & 0x1F;
        b = (*palette >> 10) & 0x1F;
	
        gray = (r * Q_8_8(0.3) + g * Q_8_8(0.59) + b * Q_8_8(0.1133)) >> 8;
	
        r = (u16)((rTone * gray)) >> 8;
        g = (u16)((gTone * gray)) >> 8;
        b = (u16)((bTone * gray)) >> 8;
	
        if (r > 31)
            r = 31;
	    
        if (g > 31)
            g = 31;
	    
        if (b > 31)
            b = 31;
	    
        *palette++ = (b << 10) | (g << 5) | (r << 0);
    }
}
