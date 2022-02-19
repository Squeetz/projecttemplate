#ifndef GUARD_TEXT_H
#define GUARD_TEXT_H

#include "global.h"

#define CHAR_SPACE          0x00
#define CHAR_0              0xA1
#define CHAR_1              0xA2
#define CHAR_2              0xA3
#define CHAR_3              0xA4
#define CHAR_4              0xA5
#define CHAR_5              0xA6
#define CHAR_6              0xA7
#define CHAR_7              0xA8
#define CHAR_8              0xA9
#define CHAR_9              0xAA
#define CHAR_EXCL_MARK      0xAB
#define CHAR_QUESTION_MARK  0xAC
#define CHAR_PERIOD         0xAD
#define CHAR_HYPHEN         0xAE
#define CHAR_ELLIPSIS       0xB0
#define CHAR_DBL_QUOT_LEFT  0xB1
#define CHAR_DBL_QUOT_RIGHT 0xB2
#define CHAR_SGL_QUOT_LEFT  0xB3
#define CHAR_SGL_QUOT_RIGHT 0xB4
#define CHAR_MALE           0xB5
#define CHAR_FEMALE         0xB6
#define CHAR_CURRENCY       0xB7
#define CHAR_COMMA          0xB8
#define CHAR_MULT_SIGN      0xB9
#define CHAR_SLASH          0xBA
#define CHAR_A              0xBB
#define CHAR_B              0xBC
#define CHAR_C              0xBD
#define CHAR_D              0xBE
#define CHAR_E              0xBF
#define CHAR_F              0xC0
#define CHAR_G              0xC1
#define CHAR_H              0xC2
#define CHAR_I              0xC3
#define CHAR_J              0xC4
#define CHAR_K              0xC5
#define CHAR_L              0xC6
#define CHAR_M              0xC7
#define CHAR_N              0xC8
#define CHAR_O              0xC9
#define CHAR_P              0xCA
#define CHAR_Q              0xCB
#define CHAR_R              0xCC
#define CHAR_S              0xCD
#define CHAR_T              0xCE
#define CHAR_U              0xCF
#define CHAR_V              0xD0
#define CHAR_W              0xD1
#define CHAR_X              0xD2
#define CHAR_Y              0xD3
#define CHAR_Z              0xD4
#define CHAR_a              0xD5
#define CHAR_b              0xD6
#define CHAR_c              0xD7
#define CHAR_d              0xD8
#define CHAR_e              0xD9
#define CHAR_f              0xDA
#define CHAR_g              0xDB
#define CHAR_h              0xDC
#define CHAR_i              0xDD
#define CHAR_j              0xDE
#define CHAR_k              0xDF
#define CHAR_l              0xE0
#define CHAR_m              0xE1
#define CHAR_n              0xE2
#define CHAR_o              0xE3
#define CHAR_p              0xE4
#define CHAR_q              0xE5
#define CHAR_r              0xE6
#define CHAR_s              0xE7
#define CHAR_t              0xE8
#define CHAR_u              0xE9
#define CHAR_v              0xEA
#define CHAR_w              0xEB
#define CHAR_x              0xEC
#define CHAR_y              0xED
#define CHAR_z              0xEE
#define CHAR_COLON          0xF0

#define CHAR_DYNAMIC_PLACEHOLDER 0xF7
#define CHAR_KEYPAD_ICON         0xF8
#define CHAR_EXTRA_EMOJI         0xF9
#define CHAR_PROMPT_SCROLL       0xFA // waits for button press and scrolls dialog
#define CHAR_PROMPT_CLEAR        0xFB // waits for button press and clears dialog
#define EXT_CTRL_CODE_BEGIN      0xFC // extended control code
#define PLACEHOLDER_BEGIN        0xFD // string placeholder
#define CHAR_NEWLINE             0xFE
#define EOS                      0xFF // end of string

// battle placeholders are located in battle_message.h
#define EXT_CTRL_CODE_COLOR                   0x1
#define EXT_CTRL_CODE_HIGHLIGHT               0x2
#define EXT_CTRL_CODE_SHADOW                  0x3
#define EXT_CTRL_CODE_COLOR_HIGHLIGHT_SHADOW  0x4
#define EXT_CTRL_CODE_PALETTE                 0x5
#define EXT_CTRL_CODE_FONT                    0x6
#define EXT_CTRL_CODE_RESET_FONT              0x7
#define EXT_CTRL_CODE_PAUSE                   0x8
#define EXT_CTRL_CODE_WAIT_BUTTON             0x9
#define EXT_CTRL_CODE_WAIT_SE                 0xA
#define EXT_CTRL_CODE_PLAY_BGM                0xB
#define EXT_CTRL_CODE_ESCAPE                  0xC
#define EXT_CTRL_CODE_SHIFT_RIGHT             0xD
#define EXT_CTRL_CODE_SHIFT_DOWN              0xE
#define EXT_CTRL_CODE_FILL_WINDOW             0xF
#define EXT_CTRL_CODE_PLAY_SE                0x10
#define EXT_CTRL_CODE_CLEAR                  0x11
#define EXT_CTRL_CODE_SKIP                   0x12
#define EXT_CTRL_CODE_CLEAR_TO               0x13
#define EXT_CTRL_CODE_MIN_LETTER_SPACING     0x14
#define EXT_CTRL_CODE_JPN                    0x15
#define EXT_CTRL_CODE_ENG                    0x16
#define EXT_CTRL_CODE_STOP_BGM               0x17
#define EXT_CTRL_CODE_RESUME_BGM             0x18

extern u8 gStringVar1[];
extern u8 gStringVar2[];
extern u8 gStringVar3[];
extern u8 gStringVar4[];

#endif // GUARD_TEXT_H
