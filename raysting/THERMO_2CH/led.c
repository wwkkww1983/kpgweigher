#include "define.h"
#include "utili.h"

#define DIGI_NUMBER     8   

/*
*       Every digital structure looks like this
*       has_point(1bit) flash(1bit) reserver(1bit)   data(5 bits)
*
*/
#define CHAR_CODE_MASK  0x1F//0x70
#define FUNC_CODE_MASK  0x40
#define POINT_ON        0x80
#define POINT_OFF       0x7F
#define FLASH_ON        0x40        
#define FLASH_OFF       0xBF
#define CHAR_0          0x00     //char 0 to char 9 is 0-9
#define CHAR_A          0x0a     //A
#define CHAR_b          0x0b     //b
#define CHAR_c          0x0c     //c

#define CHAR_d          0x0d     //d
#define CHAR_E          0x0e     //E
#define CHAR_F          0x0f     //F
#define CHAR_r          0x10     //r
#define CHAR_o          0x11     //o
#define CHAR_t          0x12     //t
#define CHAR_n          0x13     //n
#define CHAR_L		0x14	 //L
#define CHAR_Y		0x15	 //Y
#define CHAR_h		0x16	 //h
#define CHAR_U		0x17	 //U
#define CHAR_N		0x18	 //N
#define CHAR_SPACE      0x19     // 
#define CHAR_MINUS      0x1A     //- 
#define CHAR_BC         0x1B     //C
#define CHAR_MAX_NUM	0x1C

// *******************
// CH453的定义
// *******************
#define CH453_DIG0      0x6000                      //数码管位0显示
#define CH453_DIG1      0x6200		            //数码管位1显示 
#define CH453_DIG2      0x6400		            //数码管位2显示
#define CH453_DIG3      0x6600		            //数码管位3显示
#define CH453_DIG4      0x6800		            //数码管位4显示
#define CH453_DIG5      0x6A00	                    //数码管位5显示	
#define CH453_DIG6      0x6C00	                    //数码管位6显示	
#define CH453_DIG7      0x6E00		            //数码管位7显示

#define CH453_LEDA		0x7800		    //数码管位12 for led
#define CH453_LEDB		0x7A00		    //数码管位13 for led


static u8 digi_data[DIGI_NUMBER] = {0,0,0,0,0,0,0,0}; //8 digital LEDs                         

static flash u8 digi_code[CHAR_MAX_NUM] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,\ 
                                                0x77,0x7C,0x58,0x5E,0x79,0x71,0x50,\
                                                0x5C,0x78,0x54,0x38,0x6E,0x74,0x3E,0x37,0x00,0x40,0x39};
u16 reg_led;             //16 leds on off status
static u16 reg_led_flash;       //16 leds flash status
u8 flash_cnt = 0;        //counter of timer for flash

extern void Led_Output(u16);
extern void Digi_Output(u8,u8);          
extern void CH453_Write( u16 );
void digi_print(u8 *str,uchar startpos, uchar size);
extern u8 busy;
u8 needrefresh=0;
void add_refresh()
{
   needrefresh = 2;
   timer_tick();
}
                          
u8 fl_onoff = 0; //current on off status;
void timer_tick()
{
	u8 i,j,fl_cnt;
	i = 0;                      
	fl_cnt = flash_cnt;
	
	if(needrefresh == 0)
        	return;

        if(needrefresh == 2)     //direct output
                fl_cnt = 1;
        else{
                if((fl_onoff == 0) && (fl_cnt <=5)) //need on
                {
                        fl_cnt = 1;
                        fl_onoff = 1;
                }else{
                        if((fl_onoff == 1) && (fl_cnt >5)) //need off
                        {
                                fl_cnt = 6;
                                fl_onoff = 0;
                        }else{
                                return; //no action required
                        }
                }
        }
        
        if(fl_cnt != 1 && fl_cnt != 6) //1: on 6: off
                return;

        needrefresh = 0;	
        //only 1 and 6 can come here
	if(fl_cnt == 1)
	{	         
		Led_Output(reg_led | reg_led_flash);
	}else{
		Led_Output(reg_led ^ reg_led_flash);
	}
        if(reg_led_flash != 0)
                needrefresh = 1;	                

	for(i=0;i<DIGI_NUMBER;i++)
	{
	        j = digi_data[i];
		if(digi_data[i] & FLASH_ON)
		{       
		        needrefresh = 1;
			if(fl_cnt == 1)
			{
				Digi_Output(i,digi_code[CHAR_CODE_MASK & digi_data[i]] | (digi_data[i] & POINT_ON));
			}else{
				Digi_Output(i,digi_code[CHAR_SPACE] | (digi_data[i] & POINT_ON));
			}
		}else{                                         
		        j = digi_code[CHAR_CODE_MASK & digi_data[i]];
		        j = j | (digi_data[i] & POINT_ON);
		        Digi_Output(i,j);
		}
	}
}
void Digi_Output(u8 pos, u8 val)
{                                 
        while(busy == 1)
                ;
        busy = 1;     
        switch(pos)
        {
                case 4: CH453_Write(CH453_DIG0 | val); break;
                case 5: CH453_Write(CH453_DIG1 | val); break;
                case 6: CH453_Write(CH453_DIG2 | val); break;
                case 7: CH453_Write(CH453_DIG3 | val); break;
                case 0: CH453_Write(CH453_DIG4 | val); break;
                case 1: CH453_Write(CH453_DIG5 | val); break;
                case 2: CH453_Write(CH453_DIG6 | val); break;
                case 3: CH453_Write(CH453_DIG7 | val); break;
                default: break;                
        }   
        busy = 0;
}
void init_digi_Led()
{             
    reg_led = 0;
    reg_led_flash = 0;
} 
void led_on(u16 key, bool on)
{       
        
	if(on){
		reg_led |= key;
	}else{
		reg_led = reg_led & (key ^ 0xffff);
	}
	add_refresh();
}
void led_flash(u16 key, bool on)
{       
        
	if(on){
		reg_led_flash |= key;
	}else{                         
		reg_led_flash = reg_led_flash & (key ^ 0xffff);
	}
	add_refresh();
}
void digi_flash(uchar startpos, uchar endpos, bool on)
{       
        u8 i;
        for(i=0;i<DIGI_NUMBER;i++)
        {
                digi_data[i] = digi_data[i] & FLASH_OFF;        
        }
	while(startpos <= endpos)
	{
		if(on)
			digi_data[startpos] |= FLASH_ON;
		else
			digi_data[startpos] = digi_data[startpos] & FLASH_OFF;
		startpos++;
	}
	add_refresh();
}
void digi_print(u8 *str,uchar startpos, uchar size)
{     
   u8 i;       
   u8 endpos;
   endpos = startpos + size;
   if(endpos > DIGI_NUMBER)
        endpos = DIGI_NUMBER;
           
   for(i=startpos; i < endpos; i++){
      switch(*str){
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9': digi_data[i] = (digi_data[i] & (FUNC_CODE_MASK)) | (*str -'0'); break;
         case 'A': digi_data[i] = CHAR_A; break;
         case 'b': digi_data[i] = CHAR_b; break;
         case 'c': digi_data[i] = CHAR_c; break;
         case 'd': digi_data[i] = CHAR_d; break;
         case 'E': digi_data[i] = CHAR_E; break;
         case 'F': digi_data[i] = CHAR_F; break;
         case 'r': digi_data[i] = CHAR_r; break;
         case 'o': digi_data[i] = CHAR_o; break;
         case 't': digi_data[i] = CHAR_t; break;
         case 'n': digi_data[i] = CHAR_n; break;
         case 'L': digi_data[i] = CHAR_L; break; 
         case 'Y': digi_data[i] = CHAR_Y; break;          
         case 'U': digi_data[i] = CHAR_U; break;
         case 'h': digi_data[i] = CHAR_h; break;
         case 'N': digi_data[i] = CHAR_N; break;
         case 'C': digi_data[i] = CHAR_BC; break;         
         case '-': digi_data[i] = CHAR_MINUS; break;
         case '.':  digi_data[i-1] |= POINT_ON; 
               i--;
               break;
         default: digi_data[i] = CHAR_SPACE; //no show
               break;
         }
      str++;
   }
   add_refresh();
}
//const string print
void digi_cprint(u8 flash *str,uchar startpos, uchar size)
{     
   u8 i;                           
   u8 endpos;
   endpos = startpos + size;
   if(endpos > DIGI_NUMBER)
        endpos = DIGI_NUMBER;
   for(i=startpos; i < endpos; i++){
      switch(*str){
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9': digi_data[i] = (digi_data[i] & (FUNC_CODE_MASK)) | (*str -'0'); break;
         case 'A': digi_data[i] = CHAR_A; break;
         case 'b': digi_data[i] = CHAR_b; break;
         case 'c': digi_data[i] = CHAR_c; break;
         case 'd': digi_data[i] = CHAR_d; break;
         case 'E': digi_data[i] = CHAR_E; break;
         case 'F': digi_data[i] = CHAR_F; break;
         case 'r': digi_data[i] = CHAR_r; break;
         case 'o': digi_data[i] = CHAR_o; break;
         case 't': digi_data[i] = CHAR_t; break;
         case 'n': digi_data[i] = CHAR_n; break;
         case 'L': digi_data[i] = CHAR_L; break; 
         case 'Y': digi_data[i] = CHAR_Y; break;          
         case 'U': digi_data[i] = CHAR_U; break;
         case 'h': digi_data[i] = CHAR_h; break;         
         case 'N': digi_data[i] = CHAR_N; break;
         case 'C': digi_data[i] = CHAR_BC; break;
         case '-': digi_data[i] = CHAR_MINUS; break;
         case '.':  digi_data[i-1] |= POINT_ON; 
               i--;
               break;
         default: digi_data[i] = CHAR_SPACE; //no show
               break;
         }
      str++;
   }        
   add_refresh();
}             
  

void update_led(u8 mode)
{                      
        //mode 0 is for fix   sysdata.chan
        //mode 1 is for flash ch_to_search
        //D1,D2 bore, therm, D3, temp/raw
        /*D4 -- D13 PRBTYPE_PT100	        0x00
                    PRBTYPE_PT25	0x01  
                    PRBTYPE_PT1000  0x02  
                PRBTYPE_K 	   	0x03
                PRBTYPE_N		0x04
                PRBTYPE_E		0x05
                PRBTYPE_B		0x06
                PRBTYPE_J		0x07
                PRBTYPE_S		0x08
                PRBTYPE_T		0x09
                PRBTYPE_R		0x0A        
        */
        //D14,D15, ch1, ch2, minus        
        u16 dat = 0;                      
        u8 ch = 0;
        ch = ch_to_search;
        if(sysdata.mode & MODE_TEMP)
                dat = dat + LED_TEMP;                
        if(IS_BORE_MODE)        
        {
                dat = dat + LED_BORE;
                if(sysdata.type_bore == PRBTYPE_PT100)
                        dat = dat + LED_PT100;
                if(sysdata.type_bore == PRBTYPE_PT1000)
                        dat = dat + LED_PT1000;
        }
        if(IS_THERM_MODE)        
        {
                dat = dat + LED_THERM;
                if(sysdata.type_therm == PRBTYPE_K)
                        dat = dat + LED_K;
                if(sysdata.type_therm == PRBTYPE_N)
                        dat = dat + LED_N;
                if(sysdata.type_therm == PRBTYPE_E)
                        dat = dat + LED_E;
                if(sysdata.type_therm == PRBTYPE_B)
                        dat = dat + LED_B;
                if(sysdata.type_therm == PRBTYPE_J)
                        dat = dat + LED_J;
                if(sysdata.type_therm == PRBTYPE_S)
                        dat = dat + LED_S;
                if(sysdata.type_therm == PRBTYPE_T)
                        dat = dat + LED_T;
                if(sysdata.type_therm == PRBTYPE_R)
                        dat = dat + LED_R;
        }

        if(sysdata.chan == CHAN_1 || sysdata.chan == CHAN_ALL)
        {
                dat = dat + LED_CH1;
        }
        if(sysdata.chan == CHAN_2 || sysdata.chan == CHAN_ALL)
                dat = dat + LED_CH2;
        if(mode == 0)  //fix channel led
        {
                reg_led = dat;
                add_refresh();
                return;        
        }              
        //flash mode
        if(ch == 0)    
        {
                dat = dat | LED_CH1;
        }
        if(ch == 1)
        {
                dat = dat | LED_CH2;
        }
        reg_led = dat;
        add_refresh();        
        
}
void Led_Output(u16 data)
{
        u8 key;
        while(busy == 1)
                ;
        busy = 1;

        key = (u8)(data & 0x00ff);
        CH453_Write(CH453_LEDA | key);
        key = (u8)(data >> 8);
        CH453_Write(CH453_LEDB | key);
        busy = 0;
}