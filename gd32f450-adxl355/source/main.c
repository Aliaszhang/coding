
#include "gd32f4xx.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "adxl355_reg.h"
#include "systick.h"


#define SPI_CRC_ENABLE           1
#define ARRAYSIZE                10

#define SET_SPI3_NSS_HIGH          gpio_bit_set(GPIOE,GPIO_PIN_11);
#define SET_SPI3_NSS_LOW           gpio_bit_reset(GPIOE,GPIO_PIN_11);

int flag = 0;

void rcu_config(void);
void gpio_config(void);
void spi_config(void);
void uart_config(void);
int adxl355_init(void);


/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    uint8_t raw_data[0x5A] = {0};
    uint32_t acc_raw[30] = {0};
    int32_t acc_actual[30] = {0};
    int i = 0;

    systick_config();
    /* enable peripheral clock */
    rcu_config();
    /* configure GPIO */
    gpio_config();
    /* configure UART */
    uart_config();
    /* configure SPI */
    spi_config();

    SET_SPI3_NSS_HIGH

    /* enable SPI */
    spi_enable(SPI3);

    printf("start adxl355\r\n");

    adxl355_init();
    
    while(1) {

        // while (1) {
        //     spi_read_nbyte(SPI3, ADXL355_STATUS, raw_data, 1);
        //     printf("status reg = 0x%02x\r\n", raw_data[0]);
        //     if ((raw_data[0] & 0x01) == 0x01) {
        //         break;
        //     }
        //     delay_1ms(800);
        // }
        // spi_read_nbyte(SPI3, ADXL355_XDATA3, raw_data, 9);
        // for (i = 0; i < 3; i++) {
        //     acc_raw[i] = ((raw_data[i*3] << 16 | raw_data[i*3+1] << 8 | raw_data[i*3+2]) >> 4);
        //     if(acc_raw[i] > 0x80000) {
        //         acc_actual[i] = -((~acc_raw[i] & 0x3FFFF)  + 1);
        //     } else {
        //         acc_actual[i] = acc_raw[i];
        //     }
        // }
        // printf("x:%f\t y:%f\t z:%f\r\n", acc_actual[0]* 0.0039, acc_actual[1]* 0.0039, acc_actual[2]* 0.0039);

        if (flag != 0 )
        {
            flag =  0;
            spi_read_nbyte(SPI3, ADXL355_FIFO_DATA, raw_data, 0x5A);
            for (i = 0; i < 0x5A; i += 9) {
                acc_raw[i/3] = ((raw_data[i] << 16 | raw_data[i+1] << 8 | raw_data[i+2]) >> 4);
                if(acc_raw[i/3] > 0x80000) {
                    acc_actual[i/3] = -((~acc_raw[i/3] & 0x3FFFF)  + 1);
                } else {
                    acc_actual[i/3] = acc_raw[i/3];
                }
                
                acc_raw[i/3+1] = ((raw_data[i+3] << 16 | raw_data[i+4] << 8 | raw_data[i+5]) >> 4);
                if(acc_raw[i/3+1] > 0x80000) {
                    acc_actual[i/3+1] = -((~acc_raw[i/3+1] & 0x3FFFF)  + 1);
                } else {
                    acc_actual[i/3+1] = acc_raw[i/3+1];
                }
                
                acc_raw[i/3+2] = ((raw_data[i+6] << 16 | raw_data[i+7] << 8 | raw_data[i+8]) >> 4);
                if(acc_raw[i/3+2] > 0x80000) {
                    acc_actual[i/3+2] = -((~acc_raw[i/3+2] & 0x3FFFF)  + 1);
                } else {
                    acc_actual[i/3+2] = acc_raw[i/3+2];
                }

                printf("x:%f\t y:%f\t z:%f\r\n", acc_actual[i/3]* 0.0039, 
                                acc_actual[i/3+1]* 0.0039, acc_actual[i/3+2]* 0.0039);
            }
        }
    }
}


void spi_read_nbyte(uint32_t spi_periph, uint8_t addr, uint8_t *recv_ptr, uint16_t len)
{
    uint8_t addr_tmp = ((addr << 1) | 0x01);

    SET_SPI3_NSS_LOW

    while(RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TBE));
    // Send the byte
    spi_i2s_data_transmit(spi_periph, addr_tmp);
    //Wait until a data is received
    while(RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_RBNE));
    // Get the received data
    addr_tmp = spi_i2s_data_receive(spi_periph);

    addr_tmp = 0xFF;

    while(len--)
    {
        // Todo: replace register read/write by gd32f4 lib
        //Wait until the transmit buffer is empty
        while(RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TBE));
        // Send the byte
        spi_i2s_data_transmit(spi_periph, addr_tmp);

        //Wait until a data is received
        while(RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_RBNE));
        // Get the received data
        *recv_ptr++ = spi_i2s_data_receive(spi_periph);
    }

    /* release CS */
    SET_SPI3_NSS_HIGH

}

void spi_send_nbyte(uint32_t spi_periph, uint8_t addr, uint8_t *send_ptr, uint16_t len)
{
    uint8_t addr_tmp = (addr << 1);

    SET_SPI3_NSS_LOW
    while(RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TBE));
    // Send the byte
    spi_i2s_data_transmit(spi_periph, addr_tmp);
    //Wait until a data is received
    while(RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_RBNE));
    // Get the received data
    addr_tmp = spi_i2s_data_receive(spi_periph);

    while(len--)
    {
        // Todo: replace register read/write by gd32f4 lib
        //Wait until the transmit buffer is empty
        while(RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TBE));
        // Send the byte
        spi_i2s_data_transmit(spi_periph, *send_ptr++);

        //Wait until a data is received
        while(RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_RBNE));
        // Get the received data
        addr_tmp = spi_i2s_data_receive(spi_periph);
    }

    /* release CS */
    SET_SPI3_NSS_HIGH

}

int adxl355_init(void)
{
    uint8_t tmp;
    uint8_t id[5] = {0};

    spi_read_nbyte(SPI3, ADXL355_DEVID_AD, id, 4);
    printf("ADXL355 ID: (%XXL-%ox-%o), version:%d\r\n", id[0], id[1], id[2], id[3]);

    tmp = 0x52;
    spi_send_nbyte(SPI3, ADXL355_RESET, &tmp, 1);
    tmp = 0xC1;
    spi_send_nbyte(SPI3, ADXL355_RANGE, &tmp, 1);
    tmp = 0x5A;
    spi_send_nbyte(SPI3, ADXL355_FIFO_SAMPLES, &tmp, 1);
    tmp = 0x02;
    spi_send_nbyte(SPI3, ADXL355_INT_MAP, &tmp, 1);
    tmp = 0x04;
    spi_send_nbyte(SPI3, ADXL355_FILTER, &tmp, 1);
    tmp = 0x00;
    spi_send_nbyte(SPI3, ADXL355_SYNC, &tmp, 1);

    spi_read_nbyte(SPI3, ADXL355_RANGE, id, 1);
    spi_read_nbyte(SPI3, ADXL355_FIFO_SAMPLES, id+1, 1);
    spi_read_nbyte(SPI3, ADXL355_INT_MAP, id+2, 1);
    spi_read_nbyte(SPI3, ADXL355_FILTER, id+3, 1);
    spi_read_nbyte(SPI3, ADXL355_SYNC, id+4, 1);

    printf("%x(0xC1), %x(0x5A), %x(0x02), %x(0x01), %x(0x05)\r\n", id[0], id[1], id[2], id[3], id[4]);

    tmp = 0x00;
    spi_read_nbyte(SPI3, ADXL355_POWER_CTL, &tmp, 1);
    printf("Read pwrctl reg = 0x%x\r\n", tmp);
    tmp &= 0xFE; 
    spi_send_nbyte(SPI3, ADXL355_POWER_CTL, &tmp, 1);
    printf("Start xl355(0x%x)\r\n", tmp);
    spi_read_nbyte(SPI3, ADXL355_POWER_CTL, &tmp, 1);
    printf("Read xl355 pwrctl reg = 0x%x\r\n", tmp);
    if ((tmp & 0x01) != 0) {
        printf("Start xl355 failed(0x%x)\r\n",tmp);
    }

    return 0;
}



/*!
    \brief      configure different peripheral clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    rcu_periph_clock_enable(RCU_SYSCFG);

    rcu_periph_clock_enable(RCU_GPIOE);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOI);
    
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    rcu_periph_clock_enable(RCU_SPI1);
    rcu_periph_clock_enable(RCU_SPI3);
}

/*!
    \brief      configure the GPIO peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void gpio_config(void)
{
    gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_15);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);

    gpio_bit_reset(GPIOD, GPIO_PIN_15);

    /* configure the USART0 TX pin and USART0 RX pin */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);

    /* configure USART0 TX as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* configure USART0 RX as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* configure SPI3 GPIO */
    gpio_af_set(GPIOE, GPIO_AF_5, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);
    gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE,  GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);
    gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);

    gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_11);
    gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    gpio_bit_set(GPIOE, GPIO_PIN_11);

    gpio_mode_set(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_10);
    /* enable and set key EXTI interrupt to the lowest priority */
    nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);

    /* connect key EXTI line to key GPIO pin */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOE, EXTI_SOURCE_PIN10);

    /* configure key EXTI line */
    exti_init(EXTI_10, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(EXTI_10);

}

void uart_config(void)
{
    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
}

/*!
    \brief      configure the SPI peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void spi_config(void)
{
    spi_parameter_struct spi_init_struct;

    /* configure SPI3 parameter */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_8;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;

    spi_crc_off(SPI3);

    spi_init(SPI3, &spi_init_struct);
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART0, (uint8_t)ch);
    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
    return ch;
}
