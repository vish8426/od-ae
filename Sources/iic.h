
typedef unsigned char uint8_t;
typedef unsigned int uint16_t;


extern void iicinit(void);
extern int iicstart(uint8_t control);        // return non-zero if fails
extern int iicrestart(uint8_t control);      // return non-zero if fails
extern int iictransmit(uint8_t control);     // return non-zero if fails
extern void iicstop(void);
extern void iicswrcv(void);
extern int iicreceiveone(void);     // return -1 if fails, 0->255 is valid received value
extern int iicreceive(void);        // return -1 if fails, 0->255 is valid received value    
extern int iicreceivem1(void);      // return -1 if fails, 0->255 is valid received value
extern int iicreceivelast(void);    // return -1 if fails, 0->255 is valid received value
