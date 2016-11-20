#include "ili9341.h"

void ili_init(bool fast) {

  // set _dc (GPIO2) to OUTPUT, LOW
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
  GPIO_OUTPUT_SET(_dc, 0);

  // set _rst (GPIO 4) to OUTPUT, LOW
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
  GPIO_OUTPUT_SET(_rst, 0);

  // set _cs (GPIO15) to OUTPUT, LOW
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
  GPIO_OUTPUT_SET(_cs, 0);

  // init HSPI
  spi_init(HSPI);
  spi_clock(HSPI, 4, 4);  // 20mhz
  spi_tx_byte_order(HSPI, SPI_BYTE_ORDER_HIGH_TO_LOW);

  //////////////////////////////////////////////
  // Modified Adafruit initinalization wizardery


  // _rst HIGH
  GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1<<_rst);
  if(!fast) os_delay_us(5*1000);

  // _rst LOW
  GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1<<_rst);
  if(!fast) os_delay_us(20*1000);

  // _rst HIGH
  GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1<<_rst);
  if(!fast) os_delay_us(150*1000);


  spiCommandData(0xEF, 0x038002, 24);
  spiCommandData(0xCF, 0x00C130, 24);
  spiCommandData(0xED, 0x64031281, 32);
  spiCommandData(0xE8, 0x850078, 24);
  spiCommandData(0xCB, 0x392C0034, 32);
  spiCommandData(0, 0x02, 8);
  spiCommandData(0xF7, 0x20, 8);
  spiCommandData(0xEA, 0x0000, 16);
  spiCommandData(ILI9340_PWCTR1, 0x23, 8);
  spiCommandData(ILI9340_PWCTR2, 0x10, 8);
  spiCommandData(ILI9340_VMCTR1, 0x3E28, 16);
  spiCommandData(ILI9340_VMCTR2, 0x86, 8);
  spiCommandData(ILI9340_MADCTL, (ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR), 8);
  spiCommandData(ILI9340_PIXFMT, 0x55, 8);
  spiCommandData(ILI9340_FRMCTR1, 0x0018, 16);
  spiCommandData(ILI9340_DFUNCTR, 0x088227, 24);
  spiCommandData(0xF2, 0x00, 8);
  spiCommandData(ILI9340_GAMMASET, 0x01, 8);

  spiCommandData(ILI9340_GMCTRP1, 0x0F312B0C, 32);
  spiCommandData(0, 0x0E084EF1, 32);
  spiCommandData(0, 0x37071003, 32);
  spiCommandData(0, 0x0E0900, 24);

  spiCommandData(ILI9340_GMCTRN1, 0x000E1403, 32);
  spiCommandData(0, 0x110731C1, 32);
  spiCommandData(0, 0x48080F0C, 32);
  spiCommandData(0, 0x31360F, 24);

  spiCommandData(ILI9340_SLPOUT, 0, 0);
  if(!fast) os_delay_us(120*1000);
  spiCommandData(ILI9340_DISPON, 0, 0);
}




void spiCommandData(uint8 cmd, uint32 data, uint8 len) {

  if(cmd) {
    // dc low
    //if (!(GPIO_REG_READ(GPIO_OUT_ADDRESS) & BIT2))
      //GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1<<_dc);
    os_delay_us(1);
    GPIO_OUTPUT_SET(_dc, 0);

    // send 8 bit command through SPI
    spi_tx8(HSPI, cmd);

    os_delay_us(8);
  }

  // if no data return
  if(!len) return;

  // _dc high
  //if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & BIT2)
    //GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1<<_dc);
  GPIO_OUTPUT_SET(_dc, 1);

  // send all data through SPI transaction
  spi_transaction(HSPI, 0, 0, 0, 0, len, data, 0, 0);
}

/*
// len in bits
void spiCommandLgData(uint8 cmd, uint16* data, uint8 len) {

  if(cmd) {
    // _dc low
    os_delay_us(1);
    GPIO_OUTPUT_SET(_dc, 0);

    // send 8 bit command through SPI
    spi_tx8(HSPI, cmd);

    os_delay_us(8);
  }

  // if no data return
  if(!len) return;

  // _dc high
  GPIO_OUTPUT_SET(_dc, 1);



  // disable MOSI, MISO, ADDR, COMMAND, DUMMY in case previously set.
  CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI|SPI_USR_MISO|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);

  // setup bitlenghts
  WRITE_PERI_REG(SPI_USER1(HSPI), ((-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S);

  // enable MOSI function in SPI module
  SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI);

  uint8 i = (len/8) + (len%8)>0?1:0;
  // set registers from data


  while(i--)
    WRITE_PERI_REG((REG_SPI_BASE(HSPI) + 0x40 + (i*4)), data[i]);

  // spi transaction
  SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);

}
*/





void fillScreen(uint16_t color) {

  uint32_t d = ((uint32_t) color) | (((uint32_t) color) << 16);

  // write to entire screen
  spiCommandData(ILI9340_CASET, 0xEF, 32);
  spiCommandData(ILI9340_PASET, 0x13F, 32);
  spiCommandData(ILI9340_RAMWR, 0, 0);

  // _dc high since last CommandData had no actual data
  //if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & BIT2)
    //GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1<<_dc);
  GPIO_OUTPUT_SET(_dc, 1);


  // disable MOSI, MISO, ADDR, COMMAND, DUMMY in case previously set.
  CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI|SPI_USR_MISO|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);

  // setup bitlenghts
  WRITE_PERI_REG(SPI_USER1(HSPI), (((32*16)-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S);

  // enable MOSI function in SPI module
  SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI);

  // bytes to transfer (width*height*2 = total bytes) / 64 (spi transaction)
  uint32_t loop = (ILI9340_TFTWIDTH * ILI9340_TFTHEIGHT / 32);

  while(loop-- > 0) {

    // wait for SPI to be ready
    while(spi_busy(HSPI));

    // copy data to W0-W15 (16 * 32-bit = 64 bytes)
    WRITE_PERI_REG(SPI_W0(HSPI), d);
    WRITE_PERI_REG(SPI_W1(HSPI), d);
    WRITE_PERI_REG(SPI_W2(HSPI), d);
    WRITE_PERI_REG(SPI_W3(HSPI), d);
    WRITE_PERI_REG(SPI_W4(HSPI), d);
    WRITE_PERI_REG(SPI_W5(HSPI), d);
    WRITE_PERI_REG(SPI_W6(HSPI), d);
    WRITE_PERI_REG(SPI_W7(HSPI), d);
    WRITE_PERI_REG(SPI_W8(HSPI), d);
    WRITE_PERI_REG(SPI_W9(HSPI), d);
    WRITE_PERI_REG(SPI_W10(HSPI), d);
    WRITE_PERI_REG(SPI_W11(HSPI), d);
    WRITE_PERI_REG(SPI_W12(HSPI), d);
    WRITE_PERI_REG(SPI_W13(HSPI), d);
    WRITE_PERI_REG(SPI_W14(HSPI), d);
    WRITE_PERI_REG(SPI_W15(HSPI), d);

    // spi transaction
    SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);
  }

}
