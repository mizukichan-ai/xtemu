#ifndef IBM_PC_FONT_H
#define IBM_PC_FONT_H

#include <stdint.h>

/* IBM PC Character Set - 8x8 font */
/* This is a simplified version - in a full implementation, 
   you'd want to load this from a proper font file */

/* Control characters (0-31) - blank */
#define IBM_PC_CHAR_NULL      0x00
#define IBM_PC_CHAR_SOH       0x00
#define IBM_PC_CHAR_STX       0x00
#define IBM_PC_CHAR_ETX       0x00
#define IBM_PC_CHAR_EOT       0x00
#define IBM_PC_CHAR_ENQ       0x00
#define IBM_PC_CHAR_ACK       0x00
#define IBM_PC_CHAR_BEL       0x00
#define IBM_PC_CHAR_BS        0x00
#define IBM_PC_CHAR_HT        0x00
#define IBM_PC_CHAR_LF        0x00
#define IBM_PC_CHAR_VT        0x00
#define IBM_PC_CHAR_FF        0x00
#define IBM_PC_CHAR_CR        0x00
#define IBM_PC_CHAR_SO        0x00
#define IBM_PC_CHAR_SI        0x00

/* Printable characters (32-127) */
#define IBM_PC_CHAR_SPACE     0x20
#define IBM_PC_CHAR_EXCL      0x21
#define IBM_PC_CHAR_QUOT      0x22
#define IBM_PC_CHAR_HASH      0x23
#define IBM_PC_CHAR_DOLLAR    0x24
#define IBM_PC_CHAR_PERCENT   0x25
#define IBM_PC_CHAR_AMP       0x26
#define IBM_PC_CHAR_QUOT2     0x27
#define IBM_PC_CHAR_LPAREN    0x28
#define IBM_PC_CHAR_RPAREN    0x29
#define IBM_PC_CHAR_STAR      0x2A
#define IBM_PC_CHAR_PLUS      0x2B
#define IBM_PC_CHAR_COMMA     0x2C
#define IBM_PC_CHAR_MINUS     0x2D
#define IBM_PC_CHAR_DOT       0x2E
#define IBM_PC_CHAR_SLASH     0x2F
#define IBM_PC_CHAR_0         0x30
#define IBM_PC_CHAR_1         0x31
#define IBM_PC_CHAR_2         0x32
#define IBM_PC_CHAR_3         0x33
#define IBM_PC_CHAR_4         0x34
#define IBM_PC_CHAR_5         0x35
#define IBM_PC_CHAR_6         0x36
#define IBM_PC_CHAR_7         0x37
#define IBM_PC_CHAR_8         0x38
#define IBM_PC_CHAR_9         0x39
#define IBM_PC_COLON          0x3A
#define IBM_PC_CHAR_SEMICOLON 0x3B
#define IBM_PC_CHAR_LT        0x3C
#define IBM_PC_CHAR_EQ        0x3D
#define IBM_PC_CHAR_GT        0x3E
#define IBM_PC_CHAR_QMARK     0x3F
#define IBM_PC_CHAR_AT        0x40
#define IBM_PC_CHAR_A         0x41
#define IBM_PC_CHAR_B         0x42
#define IBM_PC_CHAR_C         0x43
#define IBM_PC_CHAR_D         0x44
#define IBM_PC_CHAR_E         0x45
#define IBM_PC_CHAR_F         0x46
#define IBM_PC_CHAR_G         0x47
#define IBM_PC_CHAR_H         0x48
#define IBM_PC_CHAR_I         0x49
#define IBM_PC_CHAR_J         0x4A
#define IBM_PC_CHAR_K         0x4B
#define IBM_PC_CHAR_L         0x4C
#define IBM_PC_CHAR_M         0x4D
#define IBM_PC_CHAR_N         0x4E
#define IBM_PC_CHAR_O         0x4F
#define IBM_PC_CHAR_P         0x50
#define IBM_PC_CHAR_Q         0x51
#define IBM_PC_CHAR_R         0x52
#define IBM_PC_CHAR_S         0x53
#define IBM_PC_CHAR_T         0x54
#define IBM_PC_CHAR_U         0x55
#define IBM_PC_CHAR_V         0x56
#define IBM_PC_CHAR_W         0x57
#define IBM_PC_CHAR_X         0x58
#define IBM_PC_CHAR_Y         0x59
#define IBM_PC_CHAR_Z         0x5A
#define IBM_PC_CHAR_LBRACKET  0x5B
#define IBM_PC_CHAR_BACKSLASH 0x5C
#define IBM_PC_CHAR_RBRACKET  0x5D
#define IBM_PC_CHAR_CARET     0x5E
#define IBM_PC_CHAR_UNDER     0x5F
#define IBM_PC_CHAR_GRAVE     0x60
#define IBM_PC_CHAR_a         0x61
#define IBM_PC_CHAR_b         0x62
#define IBM_PC_CHAR_c         0x63
#define IBM_PC_CHAR_d         0x64
#define IBM_PC_CHAR_e         0x65
#define IBM_PC_CHAR_f         0x66
#define IBM_PC_CHAR_g         0x67
#define IBM_PC_CHAR_h         0x68
#define IBM_PC_CHAR_i         0x69
#define IBM_PC_CHAR_j         0x6A
#define IBM_PC_CHAR_k         0x6B
#define IBM_PC_CHAR_l         0x6C
#define IBM_PC_CHAR_m         0x6D
#define IBM_PC_CHAR_n         0x6E
#define IBM_PC_CHAR_o         0x6F
#define IBM_PC_CHAR_p         0x70
#define IBM_PC_CHAR_q         0x71
#define IBM_PC_CHAR_r         0x72
#define IBM_PC_CHAR_s         0x73
#define IBM_PC_CHAR_t         0x74
#define IBM_PC_CHAR_u         0x75
#define IBM_PC_CHAR_v         0x76
#define IBM_PC_CHAR_w         0x77
#define IBM_PC_CHAR_x         0x78
#define IBM_PC_CHAR_y         0x79
#define IBM_PC_CHAR_z         0x7A
#define IBM_PC_CHAR_LBRACE    0x7B
#define IBM_PC_CHAR_PIPE      0x7C
#define IBM_PC_CHAR_RBRACE    0x7D
#define IBM_PC_CHAR_TILDE     0x7E
#define IBM_PC_CHAR_DEL       0x7F

/* Extended ASCII characters (128-255) */
#define IBM_PC_CHAR_U_A        0x80
#define IBM_PC_CHAR_U_E        0x81
#define IBM_PC_CHAR_U_I        0x82
#define IBM_PC_CHAR_U_O        0x83
#define IBM_PC_CHAR_U_U        0x84
#define IBM_PC_CHAR_U_A_A      0x85
#define IBM_PC_CHAR_U_A_E      0x86
#define IBM_PC_CHAR_U_A_I      0x87
#define IBM_PC_CHAR_U_O_U      0x88
#define IBM_PC_CHAR_U_U_U      0x89
#define IBM_PC_CHAR_D_A        0x90
#define IBM_PC_CHAR_D_E        0x91
#define IBM_PC_CHAR_D_I        0x92
#define IBM_PC_CHAR_D_O        0x93
#define IBM_PC_CHAR_D_U        0x94
#define IBM_PC_CHAR_D_A_A      0x95
#define IBM_PC_CHAR_D_A_E      0x96
#define IBM_PC_CHAR_D_A_I      0x97
#define IBM_PC_CHAR_D_O_U      0x98
#define IBM_PC_CHAR_D_U_U      0x99

/* Box drawing characters */
#define IBM_PC_CHAR_UL        0xDA
#define IBM_PC_CHAR_HORIZ     0xC4
#define IBM_PC_CHAR_UR        0xBF
#define IBM_PC_CHAR_VERT      0xB3
#define IBM_PC_CHAR_LL        0xC0
#define IBM_PC_CHAR_LR        0xD9

/* Function to get character bitmap */
const uint8_t* ibm_pc_get_char_bitmap(uint8_t character);

/* Character width and height */
#define IBM_PC_CHAR_WIDTH      8
#define IBM_PC_CHAR_HEIGHT     8

#endif /* IBM_PC_FONT_H */