#ifdef __cplusplus
extern "C" {
#endif

void set_palette(unsigned char n,unsigned char b,unsigned char r,unsigned char g);
//ƒOƒ‰ƒtƒBƒbƒN—pƒJƒ‰[ƒpƒŒƒbƒgİ’è

void pset(int x,int y,unsigned char c);
// (x,y)‚ÌˆÊ’u‚ÉƒJƒ‰[c‚Å“_‚ğ•`‰æ

void putbmpmn(int x,int y,unsigned char m,unsigned char n,const unsigned char bmp[]);
// ‰¡m*cnƒhƒbƒg‚ÌƒLƒƒƒ‰ƒNƒ^[‚ğx,y‚É•\¦
// unsigned char bmp[m*n]”z—ñ‚ÉƒJƒ‰[”Ô†
// ƒJƒ‰[”Ô†‚ª0‚Ì•”•ª‚Í“§–¾F‚Æ‚µ‚Äˆµ‚¤

void clrbmpmn(int x,int y,unsigned char m,unsigned char n);
// cm*‰¡nƒhƒbƒg‚ÌƒLƒƒƒ‰ƒNƒ^[Kè¸€
// ƒJƒ‰[0‚Å“h‚è‚Â‚Ô‚µ

void gline(int x1,int y1,int x2,int y2,unsigned char c);
// (x1,y1)-(x2,y2)‚ÉƒJƒ‰[c‚Åü•ª‚ğ•`‰æ

void hline(int x1,int x2,int y,unsigned char c);
// (x1,y)-(x2,y)‚Ö‚Ì…•½ƒ‰ƒCƒ“‚ğ‚‘¬•`‰æ

void circle(int x0,int y0,unsigned int r,unsigned char c);
// (x0,y0)‚ğ’†S‚ÉA”¼ŒarAƒJƒ‰[c‚Ì‰~‚ğ•`‰æ

void boxfill(int x1,int y1,int x2,int y2,unsigned char c);
// (x1,y1),(x2,y2)‚ğ‘ÎŠpü‚Æ‚·‚éƒJƒ‰[c‚Å“h‚ç‚ê‚½’·•ûŒ`‚ğ•`‰æ

void circlefill(int x0,int y0,unsigned int r,unsigned char c);
// (x0,y0)‚ğ’†S‚ÉA”¼ŒarAƒJƒ‰[c‚Å“h‚ç‚ê‚½‰~‚ğ•`‰æ

void putfont(int x,int y,unsigned char c,int bc,unsigned char n);
//8*8ƒhƒbƒg‚ÌƒAƒ‹ƒtƒ@ƒxƒbƒgƒtƒHƒ“ƒg•\¦
//À•Wix,y)AƒJƒ‰[”Ô†c
//bc:ƒoƒbƒNƒOƒ‰ƒ“ƒhƒJƒ‰[A•‰”‚Ìê‡–³‹
//n:•¶š”Ô†

void printstr(int x,int y,unsigned char c,int bc,unsigned char *s);
//À•W(x,y)‚©‚çƒJƒ‰[”Ô†c‚Å•¶š—ñs‚ğ•\¦Abc:ƒoƒbƒNƒOƒ‰ƒ“ƒhƒJƒ‰[

void printnum(int x,int y,unsigned char c,int bc,unsigned int n);
//À•W(x,y)‚ÉƒJƒ‰[”Ô†c‚Å”’ln‚ğ•\¦Abc:ƒoƒbƒNƒOƒ‰ƒ“ƒhƒJƒ‰[

void printnum2(int x,int y,unsigned char c,int bc,unsigned int n,unsigned char e);
//À•W(x,y)‚ÉƒJƒ‰[”Ô†c‚Å”’ln‚ğ•\¦Abc:ƒoƒbƒNƒOƒ‰ƒ“ƒhƒJƒ‰[AeŒ…‚Å•\¦

void init_graphic(void);
//ƒOƒ‰ƒtƒBƒbƒNLCDg—pŠJn

extern unsigned short palette[];
//ƒpƒŒƒbƒg—p”z—ñ

#ifdef __cplusplus
}
#endif /* End of CPP guard */
