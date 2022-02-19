#include "global.h"
#include "gflib.h"
#include "m4a.h"
#include "task.h"
#include "save.h"
#include "decompress.h"
#include "util.h"
#include "trig.h"
#include "constants/songs.h"

/*
static const u16 sCopyrightGraphicsPal[] = INCBIN_U16("graphics/intro/test.gbapal");
static const u8 sCopyrightGraphicsTiles[] = INCBIN_U8("graphics/intro/test.4bpp.lz");
static const u8 sCopyrightGraphicsMap[] = INCBIN_U8("graphics/intro/test.bin.lz");
*/

static const struct BgTemplate sBgTemplates_GameFreakScene[] = {
    {
        .bg = 3,
        .charBaseIndex = 3,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0x000
    }, {
        .bg = 2,
        .charBaseIndex = 3,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0x010
    }
};

// code
void CB2_LoadIntro(void)
{
    gMain.state++;
}

static void VBlankCB_Copyright(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}
