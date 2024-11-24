#include<kcrc.h>

/** 
  * @brief CRC calculation unit 
  */
#define CRC_BASE	(AHB1PERIPH_BASE + 0x3000UL)


typedef struct
{
  volatile uint32_t DR;         /*!< CRC Data register,             Address offset: 0x00 */
  volatile uint8_t  IDR;        /*!< CRC Independent data register, Address offset: 0x04 */
  uint8_t       RESERVED0;  /*!< Reserved, 0x05                                      */
  uint16_t      RESERVED1;  /*!< Reserved, 0x06                                      */
  volatile uint32_t CR;         /*!< CRC Control register,          Address offset: 0x08 */
}CRC_TypeDef;

#define  CRC                 ((CRC_TypeDef *) CRC_BASE)
/******************************************************************************/
/*                                                                            */
/*                          CRC calculation unit                              */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for CRC_DR register  *********************/
#define CRC_DR_DR_Pos       (0U)                                               
#define CRC_DR_DR_Msk       (0xFFFFFFFFUL << CRC_DR_DR_Pos)                     /*!< 0xFFFFFFFF */
#define CRC_DR_DR           CRC_DR_DR_Msk                                      /*!< Data register bits */


/*******************  Bit definition for CRC_IDR register  ********************/
#define CRC_IDR_IDR_Pos     (0U)                                               
#define CRC_IDR_IDR_Msk     (0xFFUL << CRC_IDR_IDR_Pos)                         /*!< 0x000000FF */
#define CRC_IDR_IDR         CRC_IDR_IDR_Msk                                    /*!< General-purpose 8-bit data register bits */


/********************  Bit definition for CRC_CR register  ********************/
#define CRC_CR_RESET_Pos    (0U)                                               
#define CRC_CR_RESET_Msk    (0x1UL << CRC_CR_RESET_Pos)                         /*!< 0x00000001 */
#define CRC_CR_RESET        CRC_CR_RESET_Msk                                   /*!< RESET bit */

//Initialize the CRC32
void CRC32_Init(void)
{
    //Enable the CRC clock
    RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
    //Reset the CRC
    CRC->CR |= CRC_CR_RESET;
}

//Process the data input it to the CRC32 data register
void CRC32_Process(uint32_t data)
{
    //Write the data to the data register
    CRC->DR = data;
}

//Finalize the CRC32 calculation
uint32_t CRC32_Finalize(uint32_t crc_data)
{ 
    //Return the XOR of the data register and the crc_data
    return CRC->DR ^ crc_data;
}

//Get the CRC32 value from the data register
uint32_t CRC32_get(void)
{   
    //Return the data register
    return CRC->DR;
}