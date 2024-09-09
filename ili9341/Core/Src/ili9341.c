#include "stm32f1xx_hal.h"
#include "ili9341.h"
#include "fsmc.h"
#include "gpio.h"
#include "fonts.h"



//����Һ��������ic�� ID
uint16_t lcdid = LCDID_UNKNOWN;

static sFONT *LCD_Currentfonts = &Font8x16;  //Ӣ������
static uint16_t CurrentTextColor   = BLACK;//ǰ��ɫ
static uint16_t CurrentBackColor   = WHITE;//����ɫ



static void                   ILI9341_REG_Config          ( void );
static void                   ILI9341_SetCursor           ( uint16_t usX, uint16_t usY );
static __inline void          ILI9341_FillColor           ( uint32_t ulAmout_Point, uint16_t usColor );

//Һ����ɨ��ģʽ����������Ҫ���ڷ���ѡ�������ļ������
//������ѡֵΪ0-7
//����ILI9341_GramScan�������÷���ʱ���Զ�����
//LCD�ճ�ʼ�����ʱ��ʹ�ñ�Ĭ��ֵ
uint8_t LCD_SCAN_MODE = 6;

//����Һ��ɨ�跽����仯��XY���ؿ���
//����ILI9341_GramScan�������÷���ʱ���Զ�����
uint16_t LCD_X_LENGTH = ILI9341_LESS_PIXEL;
uint16_t LCD_Y_LENGTH = ILI9341_MORE_PIXEL;


/**
 * @brief  �� ILI9341 д������
 * @param  usCmd : Ҫд�������Ĵ�����ַ��
 * @retval ��
 */
void ILI9341_Write_Cmd(uint8_t usCmd)
{
    // ���� DC ����Ϊ�͵�ƽ����ʾ����
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET); // DC = LOW

    // ���� CS ����Ϊ�͵�ƽ��ѡ���豸
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET); // CS = LOW

    // �� FSMC ��ַ�Ĵ���д������
    *(volatile uint16_t *)FSMC_ADDR_REG = usCmd;

    // ����豸ѡ��
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET); // CS = HIGH
}

/**
 * @brief  �� ILI9341 д������
 * @param  usCmd : Ҫд�������
 * @retval ��
 */
void ILI9341_Write_Data(uint16_t usData)
{
    // ���� DC ����Ϊ�ߵ�ƽ����ʾ����
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET); // DC = HIGH

    // ���� CS ����Ϊ�͵�ƽ��ѡ���豸
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET); // CS = LOW

    // �� FSMC ���ݼĴ���д������
    *(volatile uint16_t *)FSMC_DATA_REG = usData;

    // ����豸ѡ��
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET); // CS = HIGH
}

/**
 * @brief  �� ILI9341 ��ȡ
 * @param  ��
 * @retval ��
 */
uint16_t ILI9341_Read_Data(void)
{
    uint16_t data = 0;

    // ���� DC ����Ϊ�ߵ�ƽ����ʾ����
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET); // DC = HIGH

    // ѡ���豸
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET); // CS = LOW

    // �� FSMC ���ݼĴ�����ȡ����
    data = *(volatile uint16_t *)FSMC_DATA_REG;

    // ȡ��ѡ���豸
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET); // CS = HIGH

    return data;
}




/**
 * @brief  ���� LCD �����
 * @param  state GPIO ״̬ (GPIO_PIN_SET �� GPIO_PIN_RESET)
 * @retval ��
 */
void ILI9341_BackLed_Control(GPIO_PinState state)
{
    HAL_GPIO_WritePin(ILI9341_BK_GPIO_Port, ILI9341_BK_Pin, state);
}


/**
 * @brief  ���� LCD ������λ
 * @param  ��
 * @retval ��
 */
void ILI9341_Rst(void)
{
    // ��λ����
    HAL_GPIO_WritePin(ILI9341_RST_GPIO_Port, ILI9341_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);  // ��ʱ
    HAL_GPIO_WritePin(ILI9341_RST_GPIO_Port, ILI9341_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(100); // ��ʱ
}

/**
 * @brief  ��ȡLCD����оƬID�����������ڲ��Եײ�Ķ�д����
 * @param  ��
 * @retval ����ʱ����ֵΪLCD����оƬID: LCDID_ILI9341/LCDID_ST7789V
 *         ���򷵻�: LCDID_UNKNOWN
 */
uint16_t ILI9341_ReadID(void)
{
	uint16_t id = 0;
	
	ILI9341_Write_Cmd(0x04);
	ILI9341_Read_Data();
	ILI9341_Read_Data();
	id = ILI9341_Read_Data();
	id <<= 8;
	id |= ILI9341_Read_Data();
	
  if(id == LCDID_ST7789V)
  {
    return id;
  }
  else
  {
    ILI9341_Write_Cmd(0xD3);
    ILI9341_Read_Data();
    ILI9341_Read_Data();
    id = ILI9341_Read_Data();
    id <<= 8;
    id |= ILI9341_Read_Data();
    if(id == LCDID_ILI9341)
    {
      return id;
    }
  }
  
	return LCDID_UNKNOWN;
}

/**
 * @brief  ��ʼ��ILI9341�Ĵ���
 * @param  ��
 * @retval ��
 */
static void ILI9341_REG_Config ( void )
{
  lcdid = ILI9341_ReadID();
  
  if(lcdid == LCDID_ILI9341)
  {
    /*  Power control B (CFh)  */
    DEBUG_DELAY  ();
    ILI9341_Write_Cmd ( 0xCF  );
    ILI9341_Write_Data ( 0x00  );
    ILI9341_Write_Data ( 0x81  );
    ILI9341_Write_Data ( 0x30  );
    
    /*  Power on sequence control (EDh) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xED );
    ILI9341_Write_Data ( 0x64 );
    ILI9341_Write_Data ( 0x03 );
    ILI9341_Write_Data ( 0x12 );
    ILI9341_Write_Data ( 0x81 );
    
    /*  Driver timing control A (E8h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xE8 );
    ILI9341_Write_Data ( 0x85 );
    ILI9341_Write_Data ( 0x10 );
    ILI9341_Write_Data ( 0x78 );
    
    /*  Power control A (CBh) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xCB );
    ILI9341_Write_Data ( 0x39 );
    ILI9341_Write_Data ( 0x2C );
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0x34 );
    //ILI9341_Write_Data ( 0x02 );
    ILI9341_Write_Data ( 0x06 ); //ԭ����0x02��Ϊ0x06�ɷ�ֹҺ����ʾ����ʱ�����Ƶ����
    
    /* Pump ratio control (F7h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xF7 );
    ILI9341_Write_Data ( 0x20 );
    
    /* Driver timing control B */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xEA );
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0x00 );
    
    /* Frame Rate Control (In Normal Mode/Full Colors) (B1h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xB1 );
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0x1B );
    
    /*  Display Function Control (B6h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xB6 );
    ILI9341_Write_Data ( 0x0A );
    ILI9341_Write_Data ( 0xA2 );
    
    /* Power Control 1 (C0h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xC0 );
    ILI9341_Write_Data ( 0x35 );
    
    /* Power Control 2 (C1h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xC1 );
    ILI9341_Write_Data ( 0x11 );
    
    /* VCOM Control 1 (C5h) */
    ILI9341_Write_Cmd ( 0xC5 );
    ILI9341_Write_Data ( 0x45 );
    ILI9341_Write_Data ( 0x45 );
    
    /*  VCOM Control 2 (C7h)  */
    ILI9341_Write_Cmd ( 0xC7 );
    ILI9341_Write_Data ( 0xA2 );
    
    /* Enable 3G (F2h) */
    ILI9341_Write_Cmd ( 0xF2 );
    ILI9341_Write_Data ( 0x00 );
    
    /* Gamma Set (26h) */
    ILI9341_Write_Cmd ( 0x26 );
    ILI9341_Write_Data ( 0x01 );
    DEBUG_DELAY ();
    
    /* Positive Gamma Correction */
    ILI9341_Write_Cmd ( 0xE0 ); //Set Gamma
    ILI9341_Write_Data ( 0x0F );
    ILI9341_Write_Data ( 0x26 );
    ILI9341_Write_Data ( 0x24 );
    ILI9341_Write_Data ( 0x0B );
    ILI9341_Write_Data ( 0x0E );
    ILI9341_Write_Data ( 0x09 );
    ILI9341_Write_Data ( 0x54 );
    ILI9341_Write_Data ( 0xA8 );
    ILI9341_Write_Data ( 0x46 );
    ILI9341_Write_Data ( 0x0C );
    ILI9341_Write_Data ( 0x17 );
    ILI9341_Write_Data ( 0x09 );
    ILI9341_Write_Data ( 0x0F );
    ILI9341_Write_Data ( 0x07 );
    ILI9341_Write_Data ( 0x00 );
    
    /* Negative Gamma Correction (E1h) */
    ILI9341_Write_Cmd ( 0XE1 ); //Set Gamma
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0x19 );
    ILI9341_Write_Data ( 0x1B );
    ILI9341_Write_Data ( 0x04 );
    ILI9341_Write_Data ( 0x10 );
    ILI9341_Write_Data ( 0x07 );
    ILI9341_Write_Data ( 0x2A );
    ILI9341_Write_Data ( 0x47 );
    ILI9341_Write_Data ( 0x39 );
    ILI9341_Write_Data ( 0x03 );
    ILI9341_Write_Data ( 0x06 );
    ILI9341_Write_Data ( 0x06 );
    ILI9341_Write_Data ( 0x30 );
    ILI9341_Write_Data ( 0x38 );
    ILI9341_Write_Data ( 0x0F );
    
    /* memory access control set */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0x36 ); 	
    ILI9341_Write_Data ( 0xC8 );    /*����  ���Ͻǵ� (���)�����½� (�յ�)ɨ�跽ʽ*/
    DEBUG_DELAY ();
    
    /* column address control set */
    ILI9341_Write_Cmd ( CMD_SetCoordinateX ); 
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0xEF );
    
    /* page address control set */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( CMD_SetCoordinateY ); 
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0x01 );
    ILI9341_Write_Data ( 0x3F );
    
    /*  Pixel Format Set (3Ah)  */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0x3a ); 
    ILI9341_Write_Data ( 0x55 );
    
    /* Sleep Out (11h)  */
    ILI9341_Write_Cmd ( 0x11 );	
    HAL_Delay ( 0xAFFf<<2 );
    DEBUG_DELAY ();
    
    /* Display ON (29h) */
    ILI9341_Write_Cmd ( 0x29 ); 
	}
  
  else if(lcdid == LCDID_ST7789V)
  {
    /*  Power control B (CFh)  */
    DEBUG_DELAY  ();
    ILI9341_Write_Cmd ( 0xCF  );
    ILI9341_Write_Data ( 0x00  );
    ILI9341_Write_Data ( 0xC1  );
    ILI9341_Write_Data ( 0x30  );
    
    /*  Power on sequence control (EDh) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xED );
    ILI9341_Write_Data ( 0x64 );
    ILI9341_Write_Data ( 0x03 );
    ILI9341_Write_Data ( 0x12 );
    ILI9341_Write_Data ( 0x81 );
    
    /*  Driver timing control A (E8h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xE8 );
    ILI9341_Write_Data ( 0x85 );
    ILI9341_Write_Data ( 0x10 );
    ILI9341_Write_Data ( 0x78 );
    
    /*  Power control A (CBh) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xCB );
    ILI9341_Write_Data ( 0x39 );
    ILI9341_Write_Data ( 0x2C );
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0x34 );
    ILI9341_Write_Data ( 0x02 );
    
    /* Pump ratio control (F7h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xF7 );
    ILI9341_Write_Data ( 0x20 );
    
    /* Driver timing control B */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xEA );
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0x00 );
    
    
    /* Power Control 1 (C0h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xC0 );   //Power control
    ILI9341_Write_Data ( 0x21 );  //VRH[5:0]
    
    /* Power Control 2 (C1h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xC1 );   //Power control
    ILI9341_Write_Data ( 0x11 );  //SAP[2:0];BT[3:0]
    
    /* VCOM Control 1 (C5h) */
    ILI9341_Write_Cmd ( 0xC5 );
    ILI9341_Write_Data ( 0x2D );
    ILI9341_Write_Data ( 0x33 );
    
    /*  VCOM Control 2 (C7h)  */
//	ILI9341_Write_Cmd ( 0xC7 );
//	ILI9341_Write_Data ( 0XC0 );
    
    /* memory access control set */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0x36 );   //Memory Access Control
    ILI9341_Write_Data ( 0x00 );  /*����  ���Ͻǵ� (���)�����½� (�յ�)ɨ�跽ʽ*/
    DEBUG_DELAY ();
    
    ILI9341_Write_Cmd(0x3A);   
    ILI9341_Write_Data(0x55); 
    
      /* Frame Rate Control (In Normal Mode/Full Colors) (B1h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xB1 );
    ILI9341_Write_Data ( 0x00 );
    ILI9341_Write_Data ( 0x17 );
    
    /*  Display Function Control (B6h) */
    DEBUG_DELAY ();
    ILI9341_Write_Cmd ( 0xB6 );
    ILI9341_Write_Data ( 0x0A );
    ILI9341_Write_Data ( 0xA2 );
    
    ILI9341_Write_Cmd(0xF6);    			
    ILI9341_Write_Data(0x01); 
    ILI9341_Write_Data(0x30); 
    
    /* Enable 3G (F2h) */
    ILI9341_Write_Cmd ( 0xF2 );
    ILI9341_Write_Data ( 0x00 );
    
    /* Gamma Set (26h) */
    ILI9341_Write_Cmd ( 0x26 );
    ILI9341_Write_Data ( 0x01 );
    DEBUG_DELAY ();
    
    /* Positive Gamma Correction */
    ILI9341_Write_Cmd(0xe0); //Positive gamma
    ILI9341_Write_Data(0xd0);         
    ILI9341_Write_Data(0x00); 
    ILI9341_Write_Data(0x02); 
    ILI9341_Write_Data(0x07); 
    ILI9341_Write_Data(0x0b); 
    ILI9341_Write_Data(0x1a); 
    ILI9341_Write_Data(0x31); 
    ILI9341_Write_Data(0x54); 
    ILI9341_Write_Data(0x40); 
    ILI9341_Write_Data(0x29); 
    ILI9341_Write_Data(0x12); 
    ILI9341_Write_Data(0x12); 
    ILI9341_Write_Data(0x12); 
    ILI9341_Write_Data(0x17);

    /* Negative Gamma Correction (E1h) */
    ILI9341_Write_Cmd(0xe1); //Negative gamma
    ILI9341_Write_Data(0xd0); 
    ILI9341_Write_Data(0x00); 
    ILI9341_Write_Data(0x02); 
    ILI9341_Write_Data(0x07); 
    ILI9341_Write_Data(0x05); 
    ILI9341_Write_Data(0x25); 
    ILI9341_Write_Data(0x2d); 
    ILI9341_Write_Data(0x44); 
    ILI9341_Write_Data(0x45); 
    ILI9341_Write_Data(0x1c); 
    ILI9341_Write_Data(0x18); 
    ILI9341_Write_Data(0x16); 
    ILI9341_Write_Data(0x1c); 
    ILI9341_Write_Data(0x1d); 
  
	
//	/* column address control set */
//	ILI9341_Write_Cmd ( CMD_SetCoordinateX ); 
//	ILI9341_Write_Data ( 0x00 );
//	ILI9341_Write_Data ( 0x00 );
//	ILI9341_Write_Data ( 0x00 );
//	ILI9341_Write_Data ( 0xEF );
//	
//	/* page address control set */
//	DEBUG_DELAY ();
//	ILI9341_Write_Cmd ( CMD_SetCoordinateY ); 
//	ILI9341_Write_Data ( 0x00 );
//	ILI9341_Write_Data ( 0x00 );
//	ILI9341_Write_Data ( 0x01 );
//	ILI9341_Write_Data ( 0x3F );
	
	
    /* Sleep Out (11h)  */
    ILI9341_Write_Cmd ( 0x11 );	  //Exit Sleep
    HAL_Delay ( 0xAFFf<<2 );
    DEBUG_DELAY ();
    
    /* Display ON (29h) */
    ILI9341_Write_Cmd ( 0x29 );   //Display on
    
    ILI9341_Write_Cmd(0x2c);
  }
	
}

/**
 * @brief  ����ILI9341��GRAM��ɨ�跽�� 
 * @param  ucOption ��ѡ��GRAM��ɨ�跽�� 
 *     @arg 0-7 :������ѡֵΪ0-7��˸�����
 *
 *	����������0��3��5��6 ģʽ�ʺϴ���������ʾ���֣�
 *				���Ƽ�ʹ������ģʽ��ʾ����	����ģʽ��ʾ���ֻ��о���Ч��			
 *		
 *	����0��2��4��6 ģʽ��X��������Ϊ240��Y��������Ϊ320
 *	����1��3��5��7 ģʽ��X��������Ϊ320��Y��������Ϊ240
 *
 *	���� 6 ģʽΪ�󲿷�Һ�����̵�Ĭ����ʾ����
 *	���� 3 ģʽΪ����ͷ����ʹ�õķ���
 *	���� 0 ģʽΪBMPͼƬ��ʾ����ʹ�õķ���
 *
 * @retval ��
 * @note  ����ͼ����A��ʾ���ϣ�V��ʾ���£�<��ʾ����>��ʾ����
					X��ʾX�ᣬY��ʾY��

 *******************************************************/
void ILI9341_GramScan ( uint8_t ucOption )
{	
	//������飬ֻ������0-7
	if(ucOption >7 )
		return;
	
	//����ģʽ����LCD_SCAN_MODE��ֵ����Ҫ���ڴ�����ѡ��������
	LCD_SCAN_MODE = ucOption;
	
	//����ģʽ����XY��������ؿ���
	if(ucOption%2 == 0)	
	{
		//0 2 4 6ģʽ��X�������ؿ���Ϊ240��Y����Ϊ320
		LCD_X_LENGTH = ILI9341_LESS_PIXEL;
		LCD_Y_LENGTH =	ILI9341_MORE_PIXEL;
	}
	else				
	{
		//1 3 5 7ģʽ��X�������ؿ���Ϊ320��Y����Ϊ240
		LCD_X_LENGTH = ILI9341_MORE_PIXEL;
		LCD_Y_LENGTH =	ILI9341_LESS_PIXEL; 
	}

	//0x36��������ĸ�3λ����������GRAMɨ�跽��	
	ILI9341_Write_Cmd ( 0x36 );
  if(lcdid == LCDID_ILI9341)
  {
    ILI9341_Write_Data ( 0x08 |(ucOption<<5));//����ucOption��ֵ����LCD��������0-7��ģʽ
  }
  else if(lcdid == LCDID_ST7789V)
  {
    ILI9341_Write_Data ( 0x00 |(ucOption<<5));//����ucOption��ֵ����LCD��������0-7��ģʽ
  }
	ILI9341_Write_Cmd ( CMD_SetCoordinateX ); 
	ILI9341_Write_Data ( 0x00 );		/* x ��ʼ�����8λ */
	ILI9341_Write_Data ( 0x00 );		/* x ��ʼ�����8λ */
	ILI9341_Write_Data ( ((LCD_X_LENGTH-1)>>8)&0xFF ); /* x ���������8λ */	
	ILI9341_Write_Data ( (LCD_X_LENGTH-1)&0xFF );				/* x ���������8λ */

	ILI9341_Write_Cmd ( CMD_SetCoordinateY ); 
	ILI9341_Write_Data ( 0x00 );		/* y ��ʼ�����8λ */
	ILI9341_Write_Data ( 0x00 );		/* y ��ʼ�����8λ */
	ILI9341_Write_Data ( ((LCD_Y_LENGTH-1)>>8)&0xFF );	/* y ���������8λ */	 
	ILI9341_Write_Data ( (LCD_Y_LENGTH-1)&0xFF );				/* y ���������8λ */

	/* write gram start */
	ILI9341_Write_Cmd ( CMD_SetPixel );	
}

/**
 * @brief  ��ILI9341��ʾ����ĳһ������ĳ����ɫ��������
 * @param  usX �����ض�ɨ�跽���´��ڵ����X����
 * @param  usY �����ض�ɨ�跽���´��ڵ����Y����
 * @param  usWidth �����ڵĿ���
 * @param  usHeight �����ڵĸ߶�
 * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
 * @retval ��
 */
void ILI9341_Clear ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight )
{
	ILI9341_OpenWindow ( usX, usY, usWidth, usHeight );

	ILI9341_FillColor ( usWidth * usHeight, CurrentBackColor );		
	
}

/**
 * @brief  �趨ILI9341�Ĺ������
 * @param  usX �����ض�ɨ�跽���¹���X����
 * @param  usY �����ض�ɨ�跽���¹���Y����
 * @retval ��
 */
static void ILI9341_SetCursor ( uint16_t usX, uint16_t usY )	
{
	ILI9341_OpenWindow ( usX, usY, 1, 1 );
}

/**
  * @brief  ����LCD��ǰ��(����)��������ɫ,RGB565
  * @param  TextColor: ָ��ǰ��(����)��ɫ
  * @param  BackColor: ָ��������ɫ
  * @retval None
  */
void LCD_SetColors(uint16_t TextColor, uint16_t BackColor) 
{
  CurrentTextColor = TextColor; 
  CurrentBackColor = BackColor;
}

/**
  * @brief  ��ȡLCD��ǰ��(����)��������ɫ,RGB565
  * @param  TextColor: �����洢ǰ��(����)��ɫ��ָ�����
  * @param  BackColor: �����洢������ɫ��ָ�����
  * @retval None
  */
void LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor)
{
  *TextColor = CurrentTextColor;
  *BackColor = CurrentBackColor;
}

/**
  * @brief  ����LCD��ǰ��(����)��ɫ,RGB565
  * @param  Color: ָ��ǰ��(����)��ɫ 
  * @retval None
  */
void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

/**
  * @brief  ����LCD�ı�����ɫ,RGB565
  * @param  Color: ָ��������ɫ 
  * @retval None
  */
void LCD_SetBackColor(uint16_t Color)
{
  CurrentBackColor = Color;
}

/**
  * @brief  ����Ӣ����������
  * @param  fonts: ָ��Ҫѡ�������
	*		����Ϊ����ֵ֮һ
  * 	@arg��Font24x32;
  * 	@arg��Font16x24;
  * 	@arg��Font8x16;
  * @retval None
  */
void LCD_SetFont(sFONT *fonts)
{
  LCD_Currentfonts = fonts;
}

/**
  * @brief  ��ȡ��ǰ��������
  * @param  None.
  * @retval ���ص�ǰ��������
  */
sFONT *LCD_GetFont(void)
{
  return LCD_Currentfonts;
}

/**
  * @brief  ���ĳ������
  * @param  Line: ָ��Ҫɾ������
  *   ��������ʹ�ú�LINE(0)��LINE(1)�ȷ�ʽָ��Ҫɾ�����У�
  *   ��LINE(x)����ݵ�ǰѡ�������������Y����ֵ����ɾ����ǰ����߶ȵĵ�x�С�
  * @retval None
  */
void LCD_ClearLine(uint16_t Line)
{
  ILI9341_Clear(0,Line,LCD_X_LENGTH,((sFONT *)LCD_GetFont())->Height);	/* ��������ʾȫ�� */

}

/**
 * @brief  ��ILI9341��ʾ���Ͽ���һ������
 * @param  usX �����ض�ɨ�跽���´��ڵ����X����
 * @param  usY �����ض�ɨ�跽���´��ڵ����Y����
 * @param  usWidth �����ڵĿ���
 * @param  usHeight �����ڵĸ߶�
 * @retval ��
 */
void ILI9341_OpenWindow ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight )
{	
	ILI9341_Write_Cmd ( CMD_SetCoordinateX ); 				 /* ����X���� */
	ILI9341_Write_Data ( usX >> 8  );	 /* �ȸ�8λ��Ȼ���8λ */
	ILI9341_Write_Data ( usX & 0xff  );	 /* ������ʼ��ͽ�����*/
	ILI9341_Write_Data ( ( usX + usWidth - 1 ) >> 8  );
	ILI9341_Write_Data ( ( usX + usWidth - 1 ) & 0xff  );

	ILI9341_Write_Cmd ( CMD_SetCoordinateY ); 			     /* ����Y����*/
	ILI9341_Write_Data ( usY >> 8  );
	ILI9341_Write_Data ( usY & 0xff  );
	ILI9341_Write_Data ( ( usY + usHeight - 1 ) >> 8 );
	ILI9341_Write_Data ( ( usY + usHeight - 1) & 0xff );
	
}

/**
 * @brief  ��ILI9341��ʾ������ĳһ��ɫ������ص�
 * @param  ulAmout_Point ��Ҫ�����ɫ�����ص������Ŀ
 * @param  usColor ����ɫ
 * @retval ��
 */
static __inline void ILI9341_FillColor ( uint32_t ulAmout_Point, uint16_t usColor )
{
	uint32_t i = 0;
	
	
	/* memory write */
	ILI9341_Write_Cmd ( CMD_SetPixel );	
		
	for ( i = 0; i < ulAmout_Point; i ++ )
		ILI9341_Write_Data ( usColor );
		
}

/**
 * @brief  ��ILI9341��ʾ����ĳһ����ĳ����ɫ�������
 * @param  usX �����ض�ɨ�跽���¸õ��X����
 * @param  usY �����ض�ɨ�跽���¸õ��Y����
 * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
 * @retval ��
 */
void ILI9341_SetPointPixel ( uint16_t usX, uint16_t usY )	
{	
	if ( ( usX < LCD_X_LENGTH ) && ( usY < LCD_Y_LENGTH ) )
  {
		ILI9341_SetCursor ( usX, usY );
		
		ILI9341_FillColor ( 1, CurrentTextColor );
	}
	
}
/**
 * @brief  �� ILI9341 ��ʾ������ʾӢ���ַ���
 * @param  line �����ض�ɨ�跽�����ַ�������ʼY����
  *   ��������ʹ�ú�LINE(0)��LINE(1)�ȷ�ʽָ���������꣬
  *   ��LINE(x)����ݵ�ǰѡ�������������Y����ֵ��
	*		��ʾ������ʹ��LINE��ʱ����Ҫ��Ӣ���������ó�Font8x16
 * @param  pStr ��Ҫ��ʾ��Ӣ���ַ������׵�ַ
 * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
 * @retval ��
 */
void ILI9341_DispStringLine_EN (  uint16_t line,  char * pStr )
{
	uint16_t usX = 0;
	
	while ( * pStr != '\0' )
	{
		if ( ( usX - ILI9341_DispWindow_X_Star + LCD_Currentfonts->Width ) > LCD_X_LENGTH )
		{
			usX = ILI9341_DispWindow_X_Star;
			line += LCD_Currentfonts->Height;
		}
		
		if ( ( line - ILI9341_DispWindow_Y_Star + LCD_Currentfonts->Height ) > LCD_Y_LENGTH )
		{
			usX = ILI9341_DispWindow_X_Star;
			line = ILI9341_DispWindow_Y_Star;
		}
		
		ILI9341_DispChar_EN ( usX, line, * pStr);
		
		pStr ++;
		
		usX += LCD_Currentfonts->Width;
		
	}
	
}

/**
 * @brief  �� ILI9341 ��ʾ������ʾһ��Ӣ���ַ�
 * @param  usX �����ض�ɨ�跽�����ַ�����ʼX����
 * @param  usY �����ض�ɨ�跽���¸õ����ʼY����
 * @param  cChar ��Ҫ��ʾ��Ӣ���ַ�
 * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
 * @retval ��
 */
void ILI9341_DispChar_EN ( uint16_t usX, uint16_t usY, const char cChar )
{
	uint8_t  byteCount, bitCount,fontLength;	
	uint16_t ucRelativePositon;
	uint8_t *Pfont;
	
	//��ascii���ƫ�ƣ���ģ��������ASCII����ǰ32����ͼ�η��ţ�
	ucRelativePositon = cChar - ' ';
	
	//ÿ����ģ���ֽ���
	fontLength = (LCD_Currentfonts->Width*LCD_Currentfonts->Height)/8;
		
	//��ģ�׵�ַ
	/*ascii���ƫ��ֵ����ÿ����ģ���ֽ����������ģ��ƫ��λ��*/
	Pfont = (uint8_t *)&LCD_Currentfonts->table[ucRelativePositon * fontLength];
	
	//������ʾ����
	ILI9341_OpenWindow ( usX, usY, LCD_Currentfonts->Width, LCD_Currentfonts->Height);
	
	ILI9341_Write_Cmd ( CMD_SetPixel );			

	//���ֽڶ�ȡ��ģ����
	//����ǰ��ֱ����������ʾ���ڣ���ʾ���ݻ��Զ�����
	for ( byteCount = 0; byteCount < fontLength; byteCount++ )
	{
			//һλһλ����Ҫ��ʾ����ɫ
			for ( bitCount = 0; bitCount < 8; bitCount++ )
			{
					if ( Pfont[byteCount] & (0x80>>bitCount) )
						ILI9341_Write_Data ( CurrentTextColor );			
					else
						ILI9341_Write_Data ( CurrentBackColor );
			}	
	}	
}
/**
 * @brief  �� ILI9341 ��ʾ����ʹ�� Bresenham �㷨���߶� 
 * @param  usX1 �����ض�ɨ�跽�����߶ε�һ���˵�X����
 * @param  usY1 �����ض�ɨ�跽�����߶ε�һ���˵�Y����
 * @param  usX2 �����ض�ɨ�跽�����߶ε���һ���˵�X����
 * @param  usY2 �����ض�ɨ�跽�����߶ε���һ���˵�Y����
 * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
 * @retval ��
 */
void ILI9341_DrawLine ( uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2 )
{
	uint16_t us; 
	uint16_t usX_Current, usY_Current;
	
	int32_t lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance; 
	int32_t lIncrease_X, lIncrease_Y; 	
	
	
	lDelta_X = usX2 - usX1; //������������ 
	lDelta_Y = usY2 - usY1; 
	
	usX_Current = usX1; 
	usY_Current = usY1; 
	
	
	if ( lDelta_X > 0 ) 
		lIncrease_X = 1; //���õ������� 
	
	else if ( lDelta_X == 0 ) 
		lIncrease_X = 0;//��ֱ�� 
	
	else 
  { 
    lIncrease_X = -1;
    lDelta_X = - lDelta_X;
  } 

	
	if ( lDelta_Y > 0 )
		lIncrease_Y = 1; 
	
	else if ( lDelta_Y == 0 )
		lIncrease_Y = 0;//ˮƽ�� 
	
	else 
  {
    lIncrease_Y = -1;
    lDelta_Y = - lDelta_Y;
  } 

	
	if (  lDelta_X > lDelta_Y )
		lDistance = lDelta_X; //ѡȡ�������������� 
	
	else 
		lDistance = lDelta_Y; 

	
	for ( us = 0; us <= lDistance + 1; us ++ )//������� 
	{  
		ILI9341_SetPointPixel ( usX_Current, usY_Current );//���� 
		
		lError_X += lDelta_X ; 
		lError_Y += lDelta_Y ; 
		
		if ( lError_X > lDistance ) 
		{ 
			lError_X -= lDistance; 
			usX_Current += lIncrease_X; 
		}  
		
		if ( lError_Y > lDistance ) 
		{ 
			lError_Y -= lDistance; 
			usY_Current += lIncrease_Y; 
		} 
		
	}  
	
	
} 

/**
 * @brief  �� ILI9341 ��ʾ���ϻ�һ������
 * @param  usX_Start �����ض�ɨ�跽���¾��ε���ʼ��X����
 * @param  usY_Start �����ض�ɨ�跽���¾��ε���ʼ��Y����
 * @param  usWidth�����εĿ��ȣ���λ�����أ�
 * @param  usHeight�����εĸ߶ȣ���λ�����أ�
 * @param  ucFilled ��ѡ���Ƿ����þ���
  *   �ò���Ϊ����ֵ֮һ��
  *     @arg 0 :���ľ���
  *     @arg 1 :ʵ�ľ��� 
 * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
 * @retval ��
 */
void ILI9341_DrawRectangle ( uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight, uint8_t ucFilled )
{
	if ( ucFilled )
	{
		ILI9341_OpenWindow ( usX_Start, usY_Start, usWidth, usHeight );
		ILI9341_FillColor ( usWidth * usHeight ,CurrentTextColor);	
	}
	else
	{
		ILI9341_DrawLine ( usX_Start, usY_Start, usX_Start + usWidth - 1, usY_Start );
		ILI9341_DrawLine ( usX_Start, usY_Start + usHeight - 1, usX_Start + usWidth - 1, usY_Start + usHeight - 1 );
		ILI9341_DrawLine ( usX_Start, usY_Start, usX_Start, usY_Start + usHeight - 1 );
		ILI9341_DrawLine ( usX_Start + usWidth - 1, usY_Start, usX_Start + usWidth - 1, usY_Start + usHeight - 1 );		
	}

}


/**
 * @brief  �� ILI9341 ��ʾ����ʹ�� Bresenham �㷨��Բ
 * @param  usX_Center �����ض�ɨ�跽����Բ�ĵ�X����
 * @param  usY_Center �����ض�ɨ�跽����Բ�ĵ�Y����
 * @param  usRadius��Բ�İ뾶����λ�����أ�
 * @param  ucFilled ��ѡ���Ƿ�����Բ
  *   �ò���Ϊ����ֵ֮һ��
  *     @arg 0 :����Բ
  *     @arg 1 :ʵ��Բ
 * @note ��ʹ��LCD_SetBackColor��LCD_SetTextColor��LCD_SetColors����������ɫ
 * @retval ��
 */
void ILI9341_DrawCircle ( uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint8_t ucFilled )
{
	int16_t sCurrentX, sCurrentY;
	int16_t sError;
	
	
	sCurrentX = 0; sCurrentY = usRadius;	  
	
	sError = 3 - ( usRadius << 1 );     //�ж��¸���λ�õı�־
	
	
	while ( sCurrentX <= sCurrentY )
	{
		int16_t sCountY;
		
		
		if ( ucFilled ) 			
			for ( sCountY = sCurrentX; sCountY <= sCurrentY; sCountY ++ ) 
			{                      
				ILI9341_SetPointPixel ( usX_Center + sCurrentX, usY_Center + sCountY );           //1���о����� 
				ILI9341_SetPointPixel ( usX_Center - sCurrentX, usY_Center + sCountY );           //2       
				ILI9341_SetPointPixel ( usX_Center - sCountY,   usY_Center + sCurrentX );           //3
				ILI9341_SetPointPixel ( usX_Center - sCountY,   usY_Center - sCurrentX );           //4
				ILI9341_SetPointPixel ( usX_Center - sCurrentX, usY_Center - sCountY );           //5    
        ILI9341_SetPointPixel ( usX_Center + sCurrentX, usY_Center - sCountY );           //6
				ILI9341_SetPointPixel ( usX_Center + sCountY,   usY_Center - sCurrentX );           //7 	
        ILI9341_SetPointPixel ( usX_Center + sCountY,   usY_Center + sCurrentX );           //0				
			}
		
		else
		{          
			ILI9341_SetPointPixel ( usX_Center + sCurrentX, usY_Center + sCurrentY );             //1���о�����
			ILI9341_SetPointPixel ( usX_Center - sCurrentX, usY_Center + sCurrentY );             //2      
			ILI9341_SetPointPixel ( usX_Center - sCurrentY, usY_Center + sCurrentX );             //3
			ILI9341_SetPointPixel ( usX_Center - sCurrentY, usY_Center - sCurrentX );             //4
			ILI9341_SetPointPixel ( usX_Center - sCurrentX, usY_Center - sCurrentY );             //5       
			ILI9341_SetPointPixel ( usX_Center + sCurrentX, usY_Center - sCurrentY );             //6
			ILI9341_SetPointPixel ( usX_Center + sCurrentY, usY_Center - sCurrentX );             //7 
			ILI9341_SetPointPixel ( usX_Center + sCurrentY, usY_Center + sCurrentX );             //0
    }			
		
		
		sCurrentX ++;

		
		if ( sError < 0 ) 
			sError += 4 * sCurrentX + 6;	  
		
		else
		{
			sError += 10 + 4 * ( sCurrentX - sCurrentY );   
			sCurrentY --;
		} 	
		
		
	}
	
	
}

/**
 * @brief  ILI9341��ʼ�����������Ҫ�õ�lcd��һ��Ҫ�����������
 * @param  ��
 * @retval ��
 */
void ILI9341_Init ( void )
{
	MX_GPIO_Init();
        MX_FSMC_Init();
        
        
	ILI9341_BackLed_Control(GPIO_PIN_SET);    //����LCD�����
        ILI9341_Rst();
        ILI9341_REG_Config ();
        
        //����Ĭ��ɨ�跽������ 6 ģʽΪ�󲿷�Һ�����̵�Ĭ����ʾ����  
	ILI9341_GramScan(LCD_SCAN_MODE);

}


