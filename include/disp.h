#ifdef __cplusplus
extern "C" {
#endif

#define XWIDTH_PACMAN	14
#define YWIDTH_PACMAN	14
#define XWIDTH_MONSTER	14
#define YWIDTH_MONSTER	13
#define XWIDTH_SCORE	16
#define YWIDTH_SCORE	7

// ÉÇÉìÉXÉ^Å[î‘çÜ
#define AKABEI		0
#define AOSUKE		1
#define GUZUTA		2
#define PINKY		3

extern const unsigned char Monsterbmp[32][XWIDTH_MONSTER*YWIDTH_MONSTER];

void disp_ascii(void);
void disp_icon(void);

#ifdef __cplusplus
}
#endif /* End of CPP guard */

